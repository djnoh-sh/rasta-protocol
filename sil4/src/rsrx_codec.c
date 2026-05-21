#include "rsrx_codec.h"

static uint32_t uMessageTypeIsSupported(
	rsrx_message_type_t eMessageType)
{
	return (uint32_t)((eMessageType > RSRX_MESSAGE_TYPE_INVALID) &&
		(eMessageType <= RSRX_MESSAGE_TYPE_DIAGNOSTIC));
}

static uint32_t uReasonCodeIsSupported(
	rsrx_reason_code_t eReason)
{
	return (uint32_t)(eReason <= RSRX_REASON_INVALID_STATE_VALUE);
}

static uint32_t uTransportChannelIsSupported(
	rsrx_transport_channel_id_t eChannelId)
{
	return (uint32_t)((eChannelId > RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(eChannelId <= RSRX_TRANSPORT_CHANNEL_REDUNDANT));
}

static rsrx_event_t eMapMessageTypeToEvent(
	rsrx_message_type_t eMessageType)
{
	switch(eMessageType)
	{
		case RSRX_MESSAGE_TYPE_CONNECT_REQUEST:
			return RSRX_EVENT_VALID_INBOUND_CONNECT;

		case RSRX_MESSAGE_TYPE_CONNECT_RESPONSE:
			return RSRX_EVENT_HANDSHAKE_SUCCESS;

		case RSRX_MESSAGE_TYPE_HEARTBEAT:
			return RSRX_EVENT_VALID_HEARTBEAT;

		case RSRX_MESSAGE_TYPE_DATA:
			return RSRX_EVENT_VALID_DATA;

		case RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST:
			return RSRX_EVENT_SEQUENCE_GAP_DETECTED;

		case RSRX_MESSAGE_TYPE_DISCONNECT:
			return RSRX_EVENT_DISCONNECT_REQUEST;

		case RSRX_MESSAGE_TYPE_DIAGNOSTIC:
			return RSRX_EVENT_PROTOCOL_ERROR;

		case RSRX_MESSAGE_TYPE_INVALID:
		default:
			return RSRX_EVENT_INVALID;
	}
}

static void vWriteUint32(
	uint8_t * puBuffer,
	uint32_t uValue)
{
	puBuffer[0] = (uint8_t)((uValue >> 24) & 0xFFU);
	puBuffer[1] = (uint8_t)((uValue >> 16) & 0xFFU);
	puBuffer[2] = (uint8_t)((uValue >> 8) & 0xFFU);
	puBuffer[3] = (uint8_t)(uValue & 0xFFU);
}

static uint32_t uReadUint32(
	const uint8_t * puBuffer)
{
	return ((uint32_t)puBuffer[0] << 24) |
		((uint32_t)puBuffer[1] << 16) |
		((uint32_t)puBuffer[2] << 8) |
		(uint32_t)puBuffer[3];
}

static void vWriteUint16(
	uint8_t * puBuffer,
	uint16_t usValue)
{
	puBuffer[0] = (uint8_t)((usValue >> 8) & 0xFFU);
	puBuffer[1] = (uint8_t)(usValue & 0xFFU);
}

static uint16_t usReadUint16(
	const uint8_t * puBuffer)
{
	return (uint16_t)(((uint16_t)puBuffer[0] << 8) |
		(uint16_t)puBuffer[1]);
}

static uint32_t uReservedHeaderBytesAreZero(
	const uint8_t * puBuffer)
{
	return (uint32_t)((puBuffer[2] == 0U) &&
		(puBuffer[3] == 0U) &&
		(puBuffer[14] == 0U) &&
		(puBuffer[15] == 0U));
}

static void vClearDecodedMessage(
	rsrx_decoded_message_t * pxMessage)
{
	size_t xIndex;

	pxMessage->eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	pxMessage->eSuggestedEvent = RSRX_EVENT_INVALID;
	pxMessage->eReason = RSRX_REASON_NONE;
	pxMessage->uSequenceNumber = 0U;
	pxMessage->uConfirmationNumber = 0U;
	pxMessage->xPayloadLength = 0U;

	for(xIndex = 0U; xIndex < D_RSRX_CODEC_MAX_PAYLOAD_BYTES; ++xIndex)
	{
		pxMessage->auPayload[xIndex] = 0U;
	}
}

static uint32_t uUpdateCrc32Byte(
	uint32_t uCrc,
	uint8_t ucData)
{
	uint32_t uBitIndex;

	uCrc ^= (uint32_t)ucData;

	for(uBitIndex = 0U; uBitIndex < 8U; ++uBitIndex)
	{
		if((uCrc & 1U) != 0U)
		{
			uCrc = (uCrc >> 1U) ^ 0xEDB88320U;
		}
		else
		{
			uCrc >>= 1U;
		}
	}

	return uCrc;
}

rsrx_codec_status_t rsrx_codec_encode_message(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer)
{
	size_t xRequiredBytes;
	size_t xIndex;

	if(pxBuffer == (rsrx_encode_buffer_t *)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	pxBuffer->xEncodedLength = 0U;

	if((pxRequest == (const rsrx_encode_request_t *)0) ||
		(pxBuffer->puBuffer == (uint8_t *)0) ||
		((pxRequest->puPayload == (const uint8_t *)0) &&
			(pxRequest->xPayloadLength > 0U)))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if(uMessageTypeIsSupported(pxRequest->eMessageType) == 0U)
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE;
	}
	if(uReasonCodeIsSupported(pxRequest->eReason) == 0U)
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_REASON;
	}
	if((pxRequest->xPayloadLength > D_RSRX_CODEC_MAX_PAYLOAD_BYTES) ||
		(pxRequest->xPayloadLength > (size_t)UINT16_MAX))
	{
		return RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE;
	}

	xRequiredBytes = D_RSRX_CODEC_HEADER_BYTES + pxRequest->xPayloadLength;
	if(pxBuffer->xBufferCapacity < xRequiredBytes)
	{
		return RSRX_CODEC_STATUS_BUFFER_TOO_SMALL;
	}

	pxBuffer->puBuffer[0] = (uint8_t)pxRequest->eMessageType;
	pxBuffer->puBuffer[1] = (uint8_t)pxRequest->eReason;
	pxBuffer->puBuffer[2] = 0U;
	pxBuffer->puBuffer[3] = 0U;
	vWriteUint32(&pxBuffer->puBuffer[4], pxRequest->uSequenceNumber);
	vWriteUint32(&pxBuffer->puBuffer[8], pxRequest->uConfirmationNumber);
	vWriteUint16(&pxBuffer->puBuffer[12], (uint16_t)pxRequest->xPayloadLength);
	pxBuffer->puBuffer[14] = 0U;
	pxBuffer->puBuffer[15] = 0U;

	for(xIndex = 0U; xIndex < pxRequest->xPayloadLength; ++xIndex)
	{
		pxBuffer->puBuffer[D_RSRX_CODEC_HEADER_BYTES + xIndex] =
			pxRequest->puPayload[xIndex];
	}

	pxBuffer->xEncodedLength = xRequiredBytes;

	return RSRX_CODEC_STATUS_OK;
}

rsrx_codec_status_t rsrx_codec_decode_frame(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage)
{
	size_t xPayloadLength;
	size_t xExpectedFrameLength;
	size_t xIndex;
	rsrx_message_type_t eMessageType;

	if(pxMessage == (rsrx_decoded_message_t *)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	vClearDecodedMessage(pxMessage);

	if((pxFrame == (const rsrx_transport_frame_t *)0) ||
		(pxFrame->puPayload == (const uint8_t *)0))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if(pxFrame->xPayloadLength < D_RSRX_CODEC_HEADER_BYTES)
	{
		return RSRX_CODEC_STATUS_SHORT_HEADER;
	}
	if(pxFrame->eEventType != RSRX_TRANSPORT_EVENT_FRAME_RECEIVED)
	{
		return RSRX_CODEC_STATUS_NON_FRAME_EVENT;
	}
	if(uTransportChannelIsSupported(pxFrame->eChannelId) == 0U)
	{
		return RSRX_CODEC_STATUS_INVALID_CHANNEL;
	}

	eMessageType = (rsrx_message_type_t)pxFrame->puPayload[0];
	if(uMessageTypeIsSupported(eMessageType) == 0U)
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE;
	}
	if(uReasonCodeIsSupported((rsrx_reason_code_t)pxFrame->puPayload[1]) == 0U)
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_REASON;
	}
	if(uReservedHeaderBytesAreZero(pxFrame->puPayload) == 0U)
	{
		return RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO;
	}

	xPayloadLength = (size_t)usReadUint16(&pxFrame->puPayload[12]);
	if(xPayloadLength > D_RSRX_CODEC_MAX_PAYLOAD_BYTES)
	{
		return RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE;
	}
	xExpectedFrameLength = D_RSRX_CODEC_HEADER_BYTES + xPayloadLength;
	if(pxFrame->xPayloadLength > xExpectedFrameLength)
	{
		return RSRX_CODEC_STATUS_TRAILING_BYTES;
	}
	if(pxFrame->xPayloadLength < xExpectedFrameLength)
	{
		return RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD;
	}

	pxMessage->eMessageType = eMessageType;
	pxMessage->eSuggestedEvent = eMapMessageTypeToEvent(eMessageType);
	pxMessage->eReason = (rsrx_reason_code_t)pxFrame->puPayload[1];
	pxMessage->uSequenceNumber = uReadUint32(&pxFrame->puPayload[4]);
	pxMessage->uConfirmationNumber = uReadUint32(&pxFrame->puPayload[8]);
	pxMessage->xPayloadLength = xPayloadLength;

	for(xIndex = 0U; xIndex < xPayloadLength; ++xIndex)
	{
		pxMessage->auPayload[xIndex] =
			pxFrame->puPayload[D_RSRX_CODEC_HEADER_BYTES + xIndex];
	}

	return RSRX_CODEC_STATUS_OK;
}

const rsrx_codec_port_t * rsrx_codec_get_default_port(void)
{
	static const rsrx_codec_port_t xCodecPort =
	{
		rsrx_codec_encode_message,
		rsrx_codec_decode_frame
	};

	return &xCodecPort;
}

const rsrx_codec_port_t * rsrx_codec_get_crc32_port(void)
{
	static const rsrx_codec_port_t xCodecPort =
	{
		rsrx_codec_encode_message_with_crc32,
		rsrx_codec_decode_frame_with_crc32
	};

	return &xCodecPort;
}

const rsrx_codec_wire_profile_t * rsrx_codec_get_wire_profile(void)
{
	static const rsrx_codec_wire_profile_t xWireProfile =
	{
		D_RSRX_CODEC_WIRE_PROFILE_DEFAULT,
		D_RSRX_CODEC_WIRE_PROFILE_VERSION,
		D_RSRX_CODEC_HEADER_BYTES,
		D_RSRX_CODEC_MAX_PAYLOAD_BYTES,
		D_RSRX_CODEC_MAX_FRAME_BYTES,
		0U,
		0U,
		0U,
		0U,
		0U,
		0U
	};

	return &xWireProfile;
}

const rsrx_codec_wire_profile_t * rsrx_codec_get_crc32_wire_profile(void)
{
	static const rsrx_codec_wire_profile_t xWireProfile =
	{
		D_RSRX_CODEC_WIRE_PROFILE_CRC32,
		D_RSRX_CODEC_WIRE_PROFILE_VERSION,
		D_RSRX_CODEC_HEADER_BYTES,
		D_RSRX_CODEC_MAX_PAYLOAD_BYTES,
		D_RSRX_CODEC_MAX_CRC_FRAME_BYTES,
		D_RSRX_CODEC_CRC_BYTES,
		0U,
		0U,
		1U,
		0U,
		0U
	};

	return &xWireProfile;
}

const rsrx_codec_security_capabilities_t * rsrx_codec_get_security_capabilities(void)
{
	static const rsrx_codec_security_capabilities_t xCapabilities =
	{
		0U,
		1U,
		0U,
		0U
	};

	return &xCapabilities;
}

rsrx_codec_status_t rsrx_codec_calculate_crc32(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc)
{
	size_t xIndex;
	uint32_t uCrc = 0xFFFFFFFFU;

	if((puCrc == (uint32_t *)0) ||
		((puData == (const uint8_t *)0) && (xDataLength > 0U)))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	for(xIndex = 0U; xIndex < xDataLength; ++xIndex)
	{
		uCrc = uUpdateCrc32Byte(uCrc, puData[xIndex]);
	}

	*puCrc = ~uCrc;

	return RSRX_CODEC_STATUS_OK;
}

rsrx_codec_status_t rsrx_codec_encode_message_with_crc32(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer)
{
	return rsrx_codec_encode_message_with_crc32_calculator(
		pxRequest,
		pxBuffer,
		rsrx_codec_calculate_crc32);
}

rsrx_codec_status_t rsrx_codec_encode_message_with_crc32_calculator(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer,
	rsrx_crc32_calculate_fn pfCalculateCrc32)
{
	rsrx_codec_status_t eStatus;
	uint32_t uCrc;
	size_t xPayloadFrameLength;

	if(pxBuffer == (rsrx_encode_buffer_t *)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	pxBuffer->xEncodedLength = 0U;

	if(pfCalculateCrc32 == (rsrx_crc32_calculate_fn)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	eStatus = rsrx_codec_encode_message(pxRequest, pxBuffer);
	if(eStatus != RSRX_CODEC_STATUS_OK)
	{
		pxBuffer->xEncodedLength = 0U;
		return eStatus;
	}

	xPayloadFrameLength = pxBuffer->xEncodedLength;
	if(pxBuffer->xBufferCapacity < (xPayloadFrameLength + D_RSRX_CODEC_CRC_BYTES))
	{
		pxBuffer->xEncodedLength = 0U;
		return RSRX_CODEC_STATUS_BUFFER_TOO_SMALL;
	}

	eStatus = pfCalculateCrc32(pxBuffer->puBuffer, xPayloadFrameLength, &uCrc);
	if(eStatus != RSRX_CODEC_STATUS_OK)
	{
		pxBuffer->xEncodedLength = 0U;
		return eStatus;
	}

	vWriteUint32(&pxBuffer->puBuffer[xPayloadFrameLength], uCrc);
	pxBuffer->xEncodedLength = xPayloadFrameLength + D_RSRX_CODEC_CRC_BYTES;

	return RSRX_CODEC_STATUS_OK;
}

rsrx_codec_status_t rsrx_codec_decode_frame_with_crc32(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage)
{
	return rsrx_codec_decode_frame_with_crc32_calculator(
		pxFrame,
		pxMessage,
		rsrx_codec_calculate_crc32);
}

rsrx_codec_status_t rsrx_codec_decode_frame_with_crc32_calculator(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage,
	rsrx_crc32_calculate_fn pfCalculateCrc32)
{
	rsrx_transport_frame_t xPayloadFrame;
	uint32_t uExpectedCrc;
	uint32_t uActualCrc;
	size_t xPayloadFrameLength;
	rsrx_codec_status_t eStatus;

	if(pxMessage == (rsrx_decoded_message_t *)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	vClearDecodedMessage(pxMessage);

	if(pfCalculateCrc32 == (rsrx_crc32_calculate_fn)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if((pxFrame == (const rsrx_transport_frame_t *)0) ||
		(pxFrame->puPayload == (const uint8_t *)0))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if(pxFrame->xPayloadLength < (D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_CRC_BYTES))
	{
		return RSRX_CODEC_STATUS_CRC_TRUNCATED;
	}

	xPayloadFrameLength = pxFrame->xPayloadLength - D_RSRX_CODEC_CRC_BYTES;
	uExpectedCrc = uReadUint32(&pxFrame->puPayload[xPayloadFrameLength]);

	eStatus = pfCalculateCrc32(pxFrame->puPayload, xPayloadFrameLength, &uActualCrc);
	if(eStatus != RSRX_CODEC_STATUS_OK)
	{
		return eStatus;
	}
	if(uActualCrc != uExpectedCrc)
	{
		return RSRX_CODEC_STATUS_CRC_MISMATCH;
	}

	xPayloadFrame = *pxFrame;
	xPayloadFrame.xPayloadLength = xPayloadFrameLength;

	return rsrx_codec_decode_frame(&xPayloadFrame, pxMessage);
}
