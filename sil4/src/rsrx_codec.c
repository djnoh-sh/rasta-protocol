#include "rsrx_codec.h"

static uint32_t uMessageTypeIsSupported(
	rsrx_message_type_t eMessageType)
{
	return (uint32_t)((eMessageType > RSRX_MESSAGE_TYPE_INVALID) &&
		(eMessageType <= RSRX_MESSAGE_TYPE_DIAGNOSTIC));
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

rsrx_codec_status_t rsrx_codec_encode_message(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer)
{
	size_t xRequiredBytes;
	size_t xIndex;

	if((pxRequest == (const rsrx_encode_request_t *)0) ||
		(pxBuffer == (rsrx_encode_buffer_t *)0) ||
		(pxBuffer->puBuffer == (uint8_t *)0) ||
		((pxRequest->puPayload == (const uint8_t *)0) &&
			(pxRequest->xPayloadLength > 0U)))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if((uMessageTypeIsSupported(pxRequest->eMessageType) == 0U) ||
		(pxRequest->xPayloadLength > D_RSRX_CODEC_MAX_PAYLOAD_BYTES) ||
		(pxRequest->xPayloadLength > (size_t)UINT16_MAX))
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE;
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
	size_t xIndex;
	rsrx_message_type_t eMessageType;

	if((pxFrame == (const rsrx_transport_frame_t *)0) ||
		(pxMessage == (rsrx_decoded_message_t *)0) ||
		(pxFrame->puPayload == (const uint8_t *)0))
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	if(pxFrame->xPayloadLength < D_RSRX_CODEC_HEADER_BYTES)
	{
		return RSRX_CODEC_STATUS_DECODE_ERROR;
	}

	eMessageType = (rsrx_message_type_t)pxFrame->puPayload[0];
	if(uMessageTypeIsSupported(eMessageType) == 0U)
	{
		return RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE;
	}
	if(uReservedHeaderBytesAreZero(pxFrame->puPayload) == 0U)
	{
		return RSRX_CODEC_STATUS_DECODE_ERROR;
	}

	xPayloadLength = (size_t)usReadUint16(&pxFrame->puPayload[12]);
	if((xPayloadLength > D_RSRX_CODEC_MAX_PAYLOAD_BYTES) ||
		(pxFrame->xPayloadLength != (D_RSRX_CODEC_HEADER_BYTES + xPayloadLength)))
	{
		return RSRX_CODEC_STATUS_DECODE_ERROR;
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
