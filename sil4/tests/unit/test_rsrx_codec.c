#include <stdio.h>
#include <stdlib.h>

#include "rsrx_codec.h"

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

static void vSeedDecodedMessage(
	rsrx_decoded_message_t * pxMessage)
{
	pxMessage->eMessageType = RSRX_MESSAGE_TYPE_DATA;
	pxMessage->eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	pxMessage->eReason = RSRX_REASON_DATA_ACCEPTED;
	pxMessage->uSequenceNumber = 99U;
	pxMessage->uConfirmationNumber = 88U;
	pxMessage->xPayloadLength = 1U;
	pxMessage->auPayload[0] = 0xA5U;
}

static void vAssertDecodedMessageCleared(
	const rsrx_decoded_message_t * pxMessage,
	const char * pcMessage)
{
	vAssertTrue(pxMessage->eMessageType == RSRX_MESSAGE_TYPE_INVALID, pcMessage);
	vAssertTrue(pxMessage->eSuggestedEvent == RSRX_EVENT_INVALID, pcMessage);
	vAssertTrue(pxMessage->eReason == RSRX_REASON_NONE, pcMessage);
	vAssertTrue(pxMessage->uSequenceNumber == 0U, pcMessage);
	vAssertTrue(pxMessage->uConfirmationNumber == 0U, pcMessage);
	vAssertTrue(pxMessage->xPayloadLength == 0U, pcMessage);
	vAssertTrue(pxMessage->auPayload[0] == 0U, pcMessage);
}

static void vTestEncodeDecodeRoundTrip(void)
{
	uint8_t auPayload[5] = { 0x10U, 0x20U, 0x30U, 0x40U, 0x50U };
	uint8_t auEncoded[64];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	xRequest.eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	xRequest.uSequenceNumber = 17U;
	xRequest.uConfirmationNumber = 16U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "encode round-trip");
	vAssertTrue(xBuffer.xEncodedLength > sizeof(auPayload), "encoded length");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "decode round-trip");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_CONNECT_RESPONSE, "decoded message type");
	vAssertTrue(xMessage.eSuggestedEvent == RSRX_EVENT_HANDSHAKE_SUCCESS, "decoded event");
	vAssertTrue(xMessage.eReason == RSRX_REASON_HANDSHAKE_COMPLETED, "decoded reason");
	vAssertTrue(xMessage.uSequenceNumber == 17U, "decoded sequence");
	vAssertTrue(xMessage.uConfirmationNumber == 16U, "decoded confirmation");
	vAssertTrue(xMessage.xPayloadLength == sizeof(auPayload), "decoded payload length");
	vAssertTrue(xMessage.auPayload[4] == 0x50U, "decoded payload content");
}

static void vTestDefaultPortEncodeDecodeRoundTrip(void)
{
	const rsrx_codec_port_t * pxPort;
	uint8_t auPayload[2] = { 0xA1U, 0xB2U };
	uint8_t auEncoded[64];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	pxPort = rsrx_codec_get_default_port();
	vAssertTrue(pxPort != (const rsrx_codec_port_t *)0, "default codec port");
	vAssertTrue(pxPort->pfEncode != (rsrx_encode_message_fn)0, "default codec port encode");
	vAssertTrue(pxPort->pfDecode != (rsrx_decode_frame_fn)0, "default codec port decode");

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 3U;
	xRequest.uConfirmationNumber = 2U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(pxPort->pfEncode(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "default port encode");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(pxPort->pfDecode(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "default port decode");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_DATA, "default port decoded type");
	vAssertTrue(xMessage.eSuggestedEvent == RSRX_EVENT_VALID_DATA, "default port decoded event");
	vAssertTrue(xMessage.uSequenceNumber == 3U, "default port decoded sequence");
	vAssertTrue(xMessage.uConfirmationNumber == 2U, "default port decoded confirmation");
	vAssertTrue(xMessage.xPayloadLength == sizeof(auPayload), "default port decoded payload length");
	vAssertTrue(xMessage.auPayload[1] == 0xB2U, "default port decoded payload content");
}

static void vTestWireProfileDocumentsCurrentSecurityFields(void)
{
	const rsrx_codec_wire_profile_t * pxProfile;

	pxProfile = rsrx_codec_get_wire_profile();

	vAssertTrue(pxProfile != (const rsrx_codec_wire_profile_t *)0, "wire profile present");
	vAssertTrue(pxProfile->uProfileId == D_RSRX_CODEC_WIRE_PROFILE_DEFAULT, "wire profile default id");
	vAssertTrue(pxProfile->uProfileVersion == D_RSRX_CODEC_WIRE_PROFILE_VERSION, "wire profile version");
	vAssertTrue(pxProfile->xHeaderBytes == D_RSRX_CODEC_HEADER_BYTES, "wire profile header bytes");
	vAssertTrue(pxProfile->xMaxPayloadBytes == D_RSRX_CODEC_MAX_PAYLOAD_BYTES, "wire profile max payload bytes");
	vAssertTrue(pxProfile->xMaxFrameBytes == D_RSRX_CODEC_MAX_FRAME_BYTES, "wire profile max frame bytes");
	vAssertTrue(pxProfile->xCrcBytes == 0U, "wire profile crc bytes absent");
	vAssertTrue(pxProfile->xMacBytes == 0U, "wire profile mac bytes absent");
	vAssertTrue(pxProfile->xTimestampBytes == 0U, "wire profile timestamp bytes absent");
	vAssertTrue(pxProfile->uCrcPresent == 0U, "wire profile crc absent");
	vAssertTrue(pxProfile->uMacPresent == 0U, "wire profile mac absent");
	vAssertTrue(pxProfile->uTimestampPresent == 0U, "wire profile timestamp absent");
}

static void vTestSecurityCapabilitiesDocumentCurrentPolicy(void)
{
	const rsrx_codec_security_capabilities_t * pxCapabilities;

	pxCapabilities = rsrx_codec_get_security_capabilities();

	vAssertTrue(pxCapabilities != (const rsrx_codec_security_capabilities_t *)0, "security capabilities present");
	vAssertTrue(pxCapabilities->uDefaultCrcPresent == 0U, "security capabilities default crc absent");
	vAssertTrue(pxCapabilities->uOptionalCrc32Available == 1U, "security capabilities optional crc32 available");
	vAssertTrue(pxCapabilities->uMacAvailable == 0U, "security capabilities mac unavailable");
	vAssertTrue(pxCapabilities->uTimestampAvailable == 0U, "security capabilities timestamp unavailable");
}

static void vTestCrc32PortAndProfile(void)
{
	const rsrx_codec_port_t * pxPort;
	const rsrx_codec_wire_profile_t * pxProfile;
	uint8_t auPayload[1] = { 0x7EU };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	pxPort = rsrx_codec_get_crc32_port();
	pxProfile = rsrx_codec_get_crc32_wire_profile();

	vAssertTrue(pxPort != (const rsrx_codec_port_t *)0, "crc32 port present");
	vAssertTrue(pxPort->pfEncode == rsrx_codec_encode_message_with_crc32, "crc32 port encode binding");
	vAssertTrue(pxPort->pfDecode == rsrx_codec_decode_frame_with_crc32, "crc32 port decode binding");
	vAssertTrue(pxProfile != (const rsrx_codec_wire_profile_t *)0, "crc32 profile present");
	vAssertTrue(pxProfile->uProfileId == D_RSRX_CODEC_WIRE_PROFILE_CRC32, "crc32 profile id");
	vAssertTrue(pxProfile->uProfileVersion == D_RSRX_CODEC_WIRE_PROFILE_VERSION, "crc32 profile version");
	vAssertTrue(pxProfile->xHeaderBytes == D_RSRX_CODEC_HEADER_BYTES, "crc32 profile header bytes");
	vAssertTrue(pxProfile->xMaxPayloadBytes == D_RSRX_CODEC_MAX_PAYLOAD_BYTES, "crc32 profile max payload bytes");
	vAssertTrue(pxProfile->xMaxFrameBytes == D_RSRX_CODEC_MAX_CRC_FRAME_BYTES, "crc32 profile max frame bytes");
	vAssertTrue(pxProfile->xCrcBytes == D_RSRX_CODEC_CRC_BYTES, "crc32 profile crc bytes");
	vAssertTrue(pxProfile->xMacBytes == 0U, "crc32 profile mac bytes absent");
	vAssertTrue(pxProfile->xTimestampBytes == 0U, "crc32 profile timestamp bytes absent");
	vAssertTrue(pxProfile->uCrcPresent == 1U, "crc32 profile crc present");
	vAssertTrue(pxProfile->uMacPresent == 0U, "crc32 profile mac absent");
	vAssertTrue(pxProfile->uTimestampPresent == 0U, "crc32 profile timestamp absent");

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 13U;
	xRequest.uConfirmationNumber = 12U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(pxPort->pfEncode(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "crc32 port encode");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(pxPort->pfDecode(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "crc32 port decode");
	vAssertTrue(xMessage.uSequenceNumber == 13U, "crc32 port decoded sequence");
	vAssertTrue(xMessage.auPayload[0] == 0x7EU, "crc32 port decoded payload");
}

static void vTestCrc32PrimitiveKnownVector(void)
{
	static const uint8_t auKnownVector[] =
	{
		(uint8_t)'1',
		(uint8_t)'2',
		(uint8_t)'3',
		(uint8_t)'4',
		(uint8_t)'5',
		(uint8_t)'6',
		(uint8_t)'7',
		(uint8_t)'8',
		(uint8_t)'9'
	};
	uint32_t uCrc = 0U;

	vAssertTrue(rsrx_codec_calculate_crc32(auKnownVector, sizeof(auKnownVector), &uCrc) == RSRX_CODEC_STATUS_OK, "crc32 known vector status");
	vAssertTrue(uCrc == 0xCBF43926U, "crc32 known vector value");
}

static void vTestCrc32PrimitiveRejectsInvalidArguments(void)
{
	uint32_t uCrc = 0U;

	vAssertTrue(rsrx_codec_calculate_crc32((const uint8_t *)0, 1U, &uCrc) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "crc32 null data reject");
	vAssertTrue(rsrx_codec_calculate_crc32((const uint8_t *)0, 0U, &uCrc) == RSRX_CODEC_STATUS_OK, "crc32 empty null data accepted");
	vAssertTrue(uCrc == 0U, "crc32 empty value");
	vAssertTrue(rsrx_codec_calculate_crc32((const uint8_t *)0, 0U, (uint32_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "crc32 null output reject");
}

static void vTestCrc32WireRoundTrip(void)
{
	uint8_t auPayload[3] = { 0xCAU, 0xFEU, 0x42U };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 11U;
	xRequest.uConfirmationNumber = 10U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message_with_crc32(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "crc32 wire encode");
	vAssertTrue(xBuffer.xEncodedLength == (D_RSRX_CODEC_HEADER_BYTES + sizeof(auPayload) + D_RSRX_CODEC_CRC_BYTES), "crc32 wire encoded length");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "crc32 wire decode");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_DATA, "crc32 wire decoded type");
	vAssertTrue(xMessage.uSequenceNumber == 11U, "crc32 wire decoded sequence");
	vAssertTrue(xMessage.uConfirmationNumber == 10U, "crc32 wire decoded confirmation");
	vAssertTrue(xMessage.xPayloadLength == sizeof(auPayload), "crc32 wire decoded payload length");
	vAssertTrue(xMessage.auPayload[2] == 0x42U, "crc32 wire decoded payload content");
}

static void vTestCrc32WireRejectsSmallBuffer(void)
{
	uint8_t auPayload[1] = { 0x5AU };
	uint8_t auSmallEncoded[D_RSRX_CODEC_HEADER_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 4U;
	xRequest.uConfirmationNumber = 3U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auSmallEncoded;
	xBuffer.xBufferCapacity = sizeof(auSmallEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message_with_crc32(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_BUFFER_TOO_SMALL, "crc32 wire small buffer reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "crc32 wire small buffer clears length");
}

static void vTestCrc32WireReportsChecksumMismatch(void)
{
	uint8_t auPayload[1] = { 0x5AU };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 4U;
	xRequest.uConfirmationNumber = 3U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message_with_crc32(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "crc32 wire encode for mismatch");
	auEncoded[D_RSRX_CODEC_HEADER_BYTES] ^= 0x01U;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) == RSRX_CODEC_STATUS_CRC_MISMATCH, "crc32 wire mismatch status");
	vAssertDecodedMessageCleared(&xMessage, "crc32 mismatch clears stale decoded message");
}

static void vTestCrc32WireReportsTruncatedChecksum(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_CRC_BYTES - 1U] = { 0U };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) == RSRX_CODEC_STATUS_CRC_TRUNCATED, "crc32 wire truncated status");
	vAssertDecodedMessageCleared(&xMessage, "crc32 truncated clears stale decoded message");
}

static void vTestCrc32WireRejectsNullDecodeArguments(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_CRC_BYTES] = { 0U };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame_with_crc32((const rsrx_transport_frame_t *)0, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "crc32 null frame reject");
	vAssertDecodedMessageCleared(&xMessage, "crc32 null frame clears stale decoded message");
	vAssertTrue(rsrx_codec_decode_frame_with_crc32(&xFrame, (rsrx_decoded_message_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "crc32 null message reject");
	xFrame.puPayload = (const uint8_t *)0;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "crc32 null payload reject");
	vAssertDecodedMessageCleared(&xMessage, "crc32 null payload clears stale decoded message");
}

static void vTestCrc32WirePreservesPayloadDecodeStatus(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_CRC_BYTES] = { 0U };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;
	uint32_t uCrc;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
	auEncoded[2] = 1U;

	vAssertTrue(
		rsrx_codec_calculate_crc32(auEncoded, D_RSRX_CODEC_HEADER_BYTES, &uCrc) == RSRX_CODEC_STATUS_OK,
		"crc32 wrapped reserved-header crc calculate");
	auEncoded[D_RSRX_CODEC_HEADER_BYTES] = (uint8_t)((uCrc >> 24) & 0xFFU);
	auEncoded[D_RSRX_CODEC_HEADER_BYTES + 1U] = (uint8_t)((uCrc >> 16) & 0xFFU);
	auEncoded[D_RSRX_CODEC_HEADER_BYTES + 2U] = (uint8_t)((uCrc >> 8) & 0xFFU);
	auEncoded[D_RSRX_CODEC_HEADER_BYTES + 3U] = (uint8_t)(uCrc & 0xFFU);

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) ==
			RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO,
		"crc32 wrapped reserved-header status");
	vAssertDecodedMessageCleared(&xMessage, "crc32 wrapped reserved-header clears stale decoded message");
}

static void vTestMaxPayloadEncodeDecodeRoundTrip(void)
{
	uint8_t auPayload[D_RSRX_CODEC_MAX_PAYLOAD_BYTES];
	uint8_t auEncoded[D_RSRX_CODEC_MAX_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;
	size_t xIndex;

	for(xIndex = 0U; xIndex < sizeof(auPayload); ++xIndex)
	{
		auPayload[xIndex] = (uint8_t)(xIndex & 0xFFU);
	}

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 0x01020304U;
	xRequest.uConfirmationNumber = 0x05060708U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "max payload encode");
	vAssertTrue(xBuffer.xEncodedLength == D_RSRX_CODEC_MAX_FRAME_BYTES, "max payload encoded length");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "max payload decode");
	vAssertTrue(xMessage.xPayloadLength == sizeof(auPayload), "max payload decoded length");
	vAssertTrue(xMessage.auPayload[0] == 0x00U, "max payload first byte");
	vAssertTrue(xMessage.auPayload[D_RSRX_CODEC_MAX_PAYLOAD_BYTES - 1U] == 0xFFU, "max payload last byte");
}

static void vTestDecodeRejectsUnsupportedMessage(void)
{
	uint8_t auEncoded[16] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = 0xFFU;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "unsupported message reject");
	vAssertDecodedMessageCleared(&xMessage, "unsupported message clears stale decoded message");
}

static void vTestDecodeMapsSupportedMessageTypes(void)
{
	static const struct
	{
		rsrx_message_type_t eMessageType;
		rsrx_event_t eExpectedEvent;
	} axCases[] =
	{
		{ RSRX_MESSAGE_TYPE_CONNECT_REQUEST, RSRX_EVENT_VALID_INBOUND_CONNECT },
		{ RSRX_MESSAGE_TYPE_CONNECT_RESPONSE, RSRX_EVENT_HANDSHAKE_SUCCESS },
		{ RSRX_MESSAGE_TYPE_HEARTBEAT, RSRX_EVENT_VALID_HEARTBEAT },
		{ RSRX_MESSAGE_TYPE_DATA, RSRX_EVENT_VALID_DATA },
		{ RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST, RSRX_EVENT_SEQUENCE_GAP_DETECTED },
		{ RSRX_MESSAGE_TYPE_DISCONNECT, RSRX_EVENT_DISCONNECT_REQUEST },
		{ RSRX_MESSAGE_TYPE_DIAGNOSTIC, RSRX_EVENT_PROTOCOL_ERROR }
	};
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;
	size_t xIndex;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	for(xIndex = 0U; xIndex < (sizeof(axCases) / sizeof(axCases[0])); ++xIndex)
	{
		auEncoded[0] = (uint8_t)axCases[xIndex].eMessageType;
		auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

		vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "supported message decode");
		vAssertTrue(xMessage.eMessageType == axCases[xIndex].eMessageType, "supported message type");
		vAssertTrue(xMessage.eSuggestedEvent == axCases[xIndex].eExpectedEvent, "supported message event mapping");
	}
}

static void vTestEncodeRejectsSmallBuffer(void)
{
	uint8_t auPayload[2] = { 0x01U, 0x02U };
	uint8_t auEncoded[8];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_BUFFER_TOO_SMALL, "small buffer reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "small buffer clears stale length");
}

static void vTestEncodeFailuresClearEncodedLength(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	xRequest.eReason = RSRX_REASON_NONE;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 0U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "direct encode failure status");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "direct encode failure clears length");

	xBuffer.xEncodedLength = 99U;
	vAssertTrue(rsrx_codec_encode_message_with_crc32(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "crc32 encode failure status");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "crc32 encode failure clears length");
}

static void vTestEncodeRejectsOversizedPayloadLength(void)
{
	uint8_t auPayload[D_RSRX_CODEC_MAX_PAYLOAD_BYTES + 1U] = { 0 };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_FRAME_BYTES + 1U];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE, "oversized encode payload reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "oversized encode clears stale length");
}

static void vTestEncodeRejectsUnsupportedMessageType(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "unsupported encode message reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "unsupported encode message clears stale length");
}

static void vTestRejectsUnsupportedReasonCode(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = (rsrx_reason_code_t)0xFFU;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_REASON, "unsupported encode reason reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "unsupported encode reason clears stale length");

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = 0xFFU;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_UNSUPPORTED_REASON, "unsupported decode reason reject");
	vAssertDecodedMessageCleared(&xMessage, "unsupported reason clears stale decoded message");
}

static void vTestMaxReasonCodeEncodeDecodeRoundTrip(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DIAGNOSTIC;
	xRequest.eReason = RSRX_REASON_INVALID_STATE_VALUE;
	xRequest.uSequenceNumber = 9U;
	xRequest.uConfirmationNumber = 8U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK, "max reason encode");
	vAssertTrue(xBuffer.xEncodedLength == D_RSRX_CODEC_HEADER_BYTES, "max reason encoded length");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_OK, "max reason decode");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_DIAGNOSTIC, "max reason decoded type");
	vAssertTrue(xMessage.eReason == RSRX_REASON_INVALID_STATE_VALUE, "max reason decoded reason");
}

static void vTestDecodeRejectsReservedHeaderBytes(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
	auEncoded[2] = 1U;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO, "reserved header byte reject");
	vAssertDecodedMessageCleared(&xMessage, "reserved header tamper clears stale decoded message");
}

static void vTestDecodeRejectsReservedHeaderByteMatrix(void)
{
	static const size_t axReservedOffsets[] = { 2U, 3U, 14U, 15U };
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES];
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;
	size_t xIndex;
	size_t xByteIndex;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	for(xIndex = 0U; xIndex < (sizeof(axReservedOffsets) / sizeof(axReservedOffsets[0])); ++xIndex)
	{
		for(xByteIndex = 0U; xByteIndex < sizeof(auEncoded); ++xByteIndex)
		{
			auEncoded[xByteIndex] = 0U;
		}

		auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
		auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
		auEncoded[axReservedOffsets[xIndex]] = 1U;

		vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO, "reserved header byte matrix reject");
	}
}

static void vTestEncodeRejectsNullPayloadWithLength(void)
{
	uint8_t auEncoded[64];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 1U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null payload with length reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "null payload with length clears stale length");
}

static void vTestRejectsNullArguments(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 1U;
	xRequest.uConfirmationNumber = 1U;
	xRequest.puPayload = (const uint8_t *)0;
	xRequest.xPayloadLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_encode_message((const rsrx_encode_request_t *)0, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode request reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "null encode request clears stale length");
	vAssertTrue(rsrx_codec_encode_message(&xRequest, (rsrx_encode_buffer_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode buffer reject");
	xBuffer.puBuffer = (uint8_t *)0;
	xBuffer.xEncodedLength = 99U;
	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode output buffer reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "null encode output buffer clears stale length");
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame((const rsrx_transport_frame_t *)0, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null decode frame reject");
	vAssertDecodedMessageCleared(&xMessage, "null decode frame clears stale decoded message");
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, (rsrx_decoded_message_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null decoded message reject");
	xFrame.puPayload = (const uint8_t *)0;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null frame payload reject");
	vAssertDecodedMessageCleared(&xMessage, "null frame payload clears stale decoded message");
}

static void vTestDecodeRejectsTrailingBytes(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES + 1U] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
	auEncoded[D_RSRX_CODEC_HEADER_BYTES] = 0xA5U;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_TRAILING_BYTES, "trailing byte reject");
	vAssertDecodedMessageCleared(&xMessage, "trailing byte clears stale decoded message");
}

static void vTestDecodeRejectsShortHeader(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES - 1U] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_SHORT_HEADER, "short header reject");
	vAssertDecodedMessageCleared(&xMessage, "short header clears stale decoded message");
}

static void vTestDecodeRejectsNonFrameReceivedEvent(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_CHANNEL_DOWN;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_NON_FRAME_EVENT, "non-frame event reject");
	vAssertDecodedMessageCleared(&xMessage, "non-frame event clears stale decoded message");
}

static void vTestDecodeRejectsInvalidChannelId(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_INVALID_CHANNEL, "invalid channel reject");
	vAssertDecodedMessageCleared(&xMessage, "invalid channel clears stale decoded message");
}

static void vTestDecodeRejectsTruncatedPayload(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_HEADER_BYTES] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
	auEncoded[13] = 1U;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD, "truncated payload reject");
	vAssertDecodedMessageCleared(&xMessage, "truncated payload clears stale decoded message");
}

static void vTestDecodeRejectsOversizedDeclaredPayload(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_MAX_FRAME_BYTES + 1U] = { 0 };
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;
	auEncoded[12] = 0x02U;
	auEncoded[13] = 0x01U;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE, "oversized declared payload reject");
	vAssertDecodedMessageCleared(&xMessage, "oversized declared payload clears stale decoded message");
}

int main(void)
{
	vTestEncodeDecodeRoundTrip();
	vTestDefaultPortEncodeDecodeRoundTrip();
	vTestWireProfileDocumentsCurrentSecurityFields();
	vTestSecurityCapabilitiesDocumentCurrentPolicy();
	vTestCrc32PortAndProfile();
	vTestCrc32PrimitiveKnownVector();
	vTestCrc32PrimitiveRejectsInvalidArguments();
	vTestCrc32WireRoundTrip();
	vTestCrc32WireRejectsSmallBuffer();
	vTestCrc32WireReportsChecksumMismatch();
	vTestCrc32WireReportsTruncatedChecksum();
	vTestCrc32WireRejectsNullDecodeArguments();
	vTestCrc32WirePreservesPayloadDecodeStatus();
	vTestMaxPayloadEncodeDecodeRoundTrip();
	vTestDecodeRejectsUnsupportedMessage();
	vTestDecodeMapsSupportedMessageTypes();
	vTestEncodeRejectsSmallBuffer();
	vTestEncodeFailuresClearEncodedLength();
	vTestEncodeRejectsOversizedPayloadLength();
	vTestEncodeRejectsUnsupportedMessageType();
	vTestRejectsUnsupportedReasonCode();
	vTestMaxReasonCodeEncodeDecodeRoundTrip();
	vTestDecodeRejectsReservedHeaderBytes();
	vTestDecodeRejectsReservedHeaderByteMatrix();
	vTestEncodeRejectsNullPayloadWithLength();
	vTestRejectsNullArguments();
	vTestDecodeRejectsTrailingBytes();
	vTestDecodeRejectsShortHeader();
	vTestDecodeRejectsNonFrameReceivedEvent();
	vTestDecodeRejectsInvalidChannelId();
	vTestDecodeRejectsTruncatedPayload();
	vTestDecodeRejectsOversizedDeclaredPayload();

	(void)printf("rsrx_codec_test: all tests passed\n");

	return EXIT_SUCCESS;
}
