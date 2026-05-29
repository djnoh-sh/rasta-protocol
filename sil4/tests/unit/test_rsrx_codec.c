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

static void vSeedRastaSrDecodedPacket(
	rsrx_rasta_sr_decoded_packet_t * pxPacket)
{
	pxPacket->usPacketLength = 31U;
	pxPacket->usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	pxPacket->uReceiverId = 1U;
	pxPacket->uSenderId = 2U;
	pxPacket->uSequenceNumber = 3U;
	pxPacket->uConfirmedSequenceNumber = 4U;
	pxPacket->uTimestamp = 5U;
	pxPacket->uConfirmedTimestamp = 6U;
	pxPacket->xPayloadLength = 1U;
	pxPacket->auPayload[0] = 0xA5U;
	pxPacket->xChecksumLength = 1U;
	pxPacket->uChecksumPresent = 1U;
	pxPacket->auChecksum[0] = 0x5AU;
}

static void vAssertRastaSrDecodedPacketCleared(
	const rsrx_rasta_sr_decoded_packet_t * pxPacket,
	const char * pcMessage)
{
	vAssertTrue(pxPacket->usPacketLength == 0U, pcMessage);
	vAssertTrue(pxPacket->usMessageType == 0U, pcMessage);
	vAssertTrue(pxPacket->uReceiverId == 0U, pcMessage);
	vAssertTrue(pxPacket->uSenderId == 0U, pcMessage);
	vAssertTrue(pxPacket->uSequenceNumber == 0U, pcMessage);
	vAssertTrue(pxPacket->uConfirmedSequenceNumber == 0U, pcMessage);
	vAssertTrue(pxPacket->uTimestamp == 0U, pcMessage);
	vAssertTrue(pxPacket->uConfirmedTimestamp == 0U, pcMessage);
	vAssertTrue(pxPacket->xPayloadLength == 0U, pcMessage);
	vAssertTrue(pxPacket->auPayload[0] == 0U, pcMessage);
	vAssertTrue(pxPacket->xChecksumLength == 0U, pcMessage);
	vAssertTrue(pxPacket->uChecksumPresent == 0U, pcMessage);
	vAssertTrue(pxPacket->auChecksum[0] == 0U, pcMessage);
}

static uint32_t uInjectedCrcCallCount = 0U;

static rsrx_codec_status_t eInjectedCrc32Calculator(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc)
{
	uInjectedCrcCallCount++;
	return rsrx_codec_calculate_crc32(puData, xDataLength, puCrc);
}

static rsrx_codec_status_t eFailingInjectedCrc32Calculator(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc)
{
	(void)puData;
	(void)xDataLength;
	(void)puCrc;
	uInjectedCrcCallCount++;
	return RSRX_CODEC_STATUS_DECODE_ERROR;
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

static void vTestRastaSrWireProfileDocumentsParityTarget(void)
{
	const rsrx_codec_wire_profile_t * pxProfile;

	pxProfile = rsrx_codec_get_rasta_sr_wire_profile();

	vAssertTrue(pxProfile != (const rsrx_codec_wire_profile_t *)0, "rasta sr profile present");
	vAssertTrue(pxProfile->uProfileId == D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR, "rasta sr profile id");
	vAssertTrue(pxProfile->uProfileVersion == D_RSRX_CODEC_WIRE_PROFILE_VERSION, "rasta sr profile version");
	vAssertTrue(pxProfile->xHeaderBytes == D_RSRX_CODEC_RASTA_SR_HEADER_BYTES, "rasta sr profile header bytes");
	vAssertTrue(pxProfile->xMaxPayloadBytes == D_RSRX_CODEC_MAX_PAYLOAD_BYTES, "rasta sr profile max payload bytes");
	vAssertTrue(pxProfile->xMaxFrameBytes == D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES, "rasta sr profile max frame bytes");
	vAssertTrue(pxProfile->xCrcBytes == 0U, "rasta sr profile crc bytes absent");
	vAssertTrue(pxProfile->xMacBytes == 0U, "rasta sr profile mac bytes absent");
	vAssertTrue(pxProfile->xTimestampBytes == D_RSRX_CODEC_RASTA_SR_TIMESTAMP_BYTES, "rasta sr profile timestamp bytes");
	vAssertTrue(pxProfile->uCrcPresent == 0U, "rasta sr profile crc absent");
	vAssertTrue(pxProfile->uMacPresent == 0U, "rasta sr profile mac absent");
	vAssertTrue(pxProfile->uTimestampPresent == 1U, "rasta sr profile timestamp present");
}

static void vAssertMessageToRastaTypeMapping(
	rsrx_message_type_t eMessageType,
	uint16_t usExpectedRastaType,
	const char * pcMessage)
{
	uint16_t usMappedType = 0U;

	vAssertTrue(
		rsrx_codec_map_message_type_to_rasta_sr_type(eMessageType, &usMappedType) == RSRX_CODEC_STATUS_OK,
		pcMessage);
	vAssertTrue(usMappedType == usExpectedRastaType, pcMessage);
}

static void vAssertRastaTypeToMessageMapping(
	uint16_t usRastaType,
	rsrx_message_type_t eExpectedMessageType,
	const char * pcMessage)
{
	rsrx_message_type_t eMappedType = RSRX_MESSAGE_TYPE_INVALID;

	vAssertTrue(
		rsrx_codec_map_rasta_sr_type_to_message_type(usRastaType, &eMappedType) == RSRX_CODEC_STATUS_OK,
		pcMessage);
	vAssertTrue(eMappedType == eExpectedMessageType, pcMessage);
}

static void vAssertReasonToRastaDisconnectMapping(
	rsrx_reason_code_t eReason,
	uint16_t usExpectedRastaReason,
	const char * pcMessage)
{
	uint16_t usMappedReason = 0U;

	vAssertTrue(
		rsrx_codec_map_reason_to_rasta_disconnect_reason(eReason, &usMappedReason) == RSRX_CODEC_STATUS_OK,
		pcMessage);
	vAssertTrue(usMappedReason == usExpectedRastaReason, pcMessage);
}

static void vTestRastaSrMessageTypeMapping(void)
{
	uint16_t usMappedType = 0xFFFFU;
	rsrx_message_type_t eMappedType = RSRX_MESSAGE_TYPE_DATA;

	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_CONNECT_REQUEST, (uint16_t)RSRX_RASTA_SR_TYPE_CONNREQ, "connreq outbound mapping");
	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_CONNECT_RESPONSE, (uint16_t)RSRX_RASTA_SR_TYPE_CONNRESP, "connresp outbound mapping");
	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_HEARTBEAT, (uint16_t)RSRX_RASTA_SR_TYPE_HB, "heartbeat outbound mapping");
	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_DATA, (uint16_t)RSRX_RASTA_SR_TYPE_DATA, "data outbound mapping");
	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST, (uint16_t)RSRX_RASTA_SR_TYPE_RETRREQ, "retrreq outbound mapping");
	vAssertMessageToRastaTypeMapping(RSRX_MESSAGE_TYPE_DISCONNECT, (uint16_t)RSRX_RASTA_SR_TYPE_DISCREQ, "discreq outbound mapping");

	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_CONNREQ, RSRX_MESSAGE_TYPE_CONNECT_REQUEST, "connreq inbound mapping");
	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_CONNRESP, RSRX_MESSAGE_TYPE_CONNECT_RESPONSE, "connresp inbound mapping");
	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_HB, RSRX_MESSAGE_TYPE_HEARTBEAT, "heartbeat inbound mapping");
	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_DATA, RSRX_MESSAGE_TYPE_DATA, "data inbound mapping");
	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_RETRREQ, RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST, "retrreq inbound mapping");
	vAssertRastaTypeToMessageMapping((uint16_t)RSRX_RASTA_SR_TYPE_DISCREQ, RSRX_MESSAGE_TYPE_DISCONNECT, "discreq inbound mapping");

	vAssertTrue(
		rsrx_codec_map_message_type_to_rasta_sr_type(RSRX_MESSAGE_TYPE_DIAGNOSTIC, &usMappedType) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		"diagnostic unsupported outbound mapping");
	vAssertTrue(usMappedType == 0U, "diagnostic unsupported clears mapped rasta type");
	vAssertTrue(
		rsrx_codec_map_rasta_sr_type_to_message_type((uint16_t)RSRX_RASTA_SR_TYPE_RETRRESP, &eMappedType) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		"retrresp unsupported inbound mapping");
	vAssertTrue(eMappedType == RSRX_MESSAGE_TYPE_INVALID, "retrresp unsupported clears mapped message type");
	vAssertTrue(
		rsrx_codec_map_message_type_to_rasta_sr_type(RSRX_MESSAGE_TYPE_DATA, (uint16_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"null rasta type output reject");
	vAssertTrue(
		rsrx_codec_map_rasta_sr_type_to_message_type((uint16_t)RSRX_RASTA_SR_TYPE_DATA, (rsrx_message_type_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"null message output reject");
}

static void vTestRastaDisconnectReasonMapping(void)
{
	uint16_t usMappedReason = 0xFFFFU;

	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_DISCONNECT_REQUESTED, (uint16_t)RSRX_RASTA_DISC_REASON_USERREQUEST, "disconnect user request mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_SHUTDOWN_REQUESTED, (uint16_t)RSRX_RASTA_DISC_REASON_USERREQUEST, "shutdown user request mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_INVALID_MESSAGE_RECEIVED, (uint16_t)RSRX_RASTA_DISC_REASON_UNEXPECTEDTYPE, "invalid message mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_INVALID_RESPONSE_RECEIVED, (uint16_t)RSRX_RASTA_DISC_REASON_UNEXPECTEDTYPE, "invalid response mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_SEQUENCE_GAP_DETECTED, (uint16_t)RSRX_RASTA_DISC_REASON_SEQNERROR, "sequence error mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_TIMEOUT_EXPIRED, (uint16_t)RSRX_RASTA_DISC_REASON_TIMEOUT, "timeout mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_INVALID_INPUT_ARGUMENT, (uint16_t)RSRX_RASTA_DISC_REASON_SERVICENOTALLOWED, "service not allowed mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_VERSION_MISMATCH_DETECTED, (uint16_t)RSRX_RASTA_DISC_REASON_INCOMPATIBLEVERSION, "incompatible version mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_RETRANSMISSION_FAILED, (uint16_t)RSRX_RASTA_DISC_REASON_RETRFAILED, "retransmission failed mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_PROTOCOL_ERROR_DETECTED, (uint16_t)RSRX_RASTA_DISC_REASON_PROTOCOLERROR, "protocol error mapping");
	vAssertReasonToRastaDisconnectMapping(RSRX_REASON_CONSERVATIVE_FAILSAFE, (uint16_t)RSRX_RASTA_DISC_REASON_PROTOCOLERROR, "failsafe protocol error mapping");

	vAssertTrue(
		rsrx_codec_map_reason_to_rasta_disconnect_reason(RSRX_REASON_HEARTBEAT_ACCEPTED, &usMappedReason) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_REASON,
		"non-disconnect reason unsupported mapping");
	vAssertTrue(usMappedReason == 0U, "unsupported reason clears mapped rasta reason");
	vAssertTrue(
		rsrx_codec_map_reason_to_rasta_disconnect_reason(RSRX_REASON_TIMEOUT_EXPIRED, (uint16_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"null disconnect reason output reject");
}

static void vTestRastaSrByteOrderIsFixedBigEndian(void)
{
	uint8_t au16[2] = { 0U, 0U };
	uint8_t au32[4] = { 0U, 0U, 0U, 0U };
	uint16_t usDecoded16 = 0U;
	uint32_t uDecoded32 = 0U;

	vAssertTrue(D_RSRX_CODEC_RASTA_SR_BYTE_ORDER_BIG_ENDIAN == 1U, "rasta sr byte order constant");
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x1830U, au16) == RSRX_CODEC_STATUS_OK,
		"rasta sr write uint16");
	vAssertTrue(au16[0] == 0x18U, "rasta sr uint16 high byte");
	vAssertTrue(au16[1] == 0x30U, "rasta sr uint16 low byte");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint16(au16, &usDecoded16) == RSRX_CODEC_STATUS_OK,
		"rasta sr read uint16");
	vAssertTrue(usDecoded16 == 0x1830U, "rasta sr uint16 round-trip");

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint32(0x11223344U, au32) == RSRX_CODEC_STATUS_OK,
		"rasta sr write uint32");
	vAssertTrue(au32[0] == 0x11U, "rasta sr uint32 byte 0");
	vAssertTrue(au32[1] == 0x22U, "rasta sr uint32 byte 1");
	vAssertTrue(au32[2] == 0x33U, "rasta sr uint32 byte 2");
	vAssertTrue(au32[3] == 0x44U, "rasta sr uint32 byte 3");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint32(au32, &uDecoded32) == RSRX_CODEC_STATUS_OK,
		"rasta sr read uint32");
	vAssertTrue(uDecoded32 == 0x11223344U, "rasta sr uint32 round-trip");

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x0102U, (uint8_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr write uint16 null reject");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint16((const uint8_t *)0, &usDecoded16) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr read uint16 null buffer reject");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint16(au16, (uint16_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr read uint16 null output reject");
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint32(0x01020304U, (uint8_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr write uint32 null reject");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint32((const uint8_t *)0, &uDecoded32) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr read uint32 null buffer reject");
	vAssertTrue(
		rsrx_codec_read_rasta_sr_uint32(au32, (uint32_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr read uint32 null output reject");
}

static void vTestRastaSrNoChecksumEncodeDecodeRoundTrip(void)
{
	static const uint8_t auPayload[3] = { 0xDEU, 0xADU, 0x42U };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES];
	rsrx_rasta_sr_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_rasta_sr_decoded_packet_t xPacket;

	xRequest.usPacketLength = (uint16_t)(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + sizeof(auPayload));
	xRequest.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xRequest.uReceiverId = 0x11223344U;
	xRequest.uSenderId = 0x55667788U;
	xRequest.uSequenceNumber = 0x01020304U;
	xRequest.uConfirmedSequenceNumber = 0x05060708U;
	xRequest.uTimestamp = 0x10203040U;
	xRequest.uConfirmedTimestamp = 0x50607080U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);
	xRequest.puChecksum = (const uint8_t *)0;
	xRequest.xChecksumLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK,
		"rasta sr no-checksum encode");
	vAssertTrue(xBuffer.xEncodedLength == xRequest.usPacketLength, "rasta sr no-checksum encoded length");
	vAssertTrue(auEncoded[0] == 0x00U, "rasta sr length high byte");
	vAssertTrue(auEncoded[1] == 0x1FU, "rasta sr length low byte");
	vAssertTrue(auEncoded[2] == 0x18U, "rasta sr type high byte");
	vAssertTrue(auEncoded[3] == 0x60U, "rasta sr type low byte");
	vAssertTrue(auEncoded[4] == 0x11U, "rasta sr receiver byte 0");
	vAssertTrue(auEncoded[7] == 0x44U, "rasta sr receiver byte 3");
	vAssertTrue(auEncoded[8] == 0x55U, "rasta sr sender byte 0");
	vAssertTrue(auEncoded[11] == 0x88U, "rasta sr sender byte 3");
	vAssertTrue(auEncoded[12] == 0x01U, "rasta sr sequence byte 0");
	vAssertTrue(auEncoded[15] == 0x04U, "rasta sr sequence byte 3");
	vAssertTrue(auEncoded[16] == 0x05U, "rasta sr confirmed sequence byte 0");
	vAssertTrue(auEncoded[19] == 0x08U, "rasta sr confirmed sequence byte 3");
	vAssertTrue(auEncoded[20] == 0x10U, "rasta sr timestamp byte 0");
	vAssertTrue(auEncoded[23] == 0x40U, "rasta sr timestamp byte 3");
	vAssertTrue(auEncoded[24] == 0x50U, "rasta sr confirmed timestamp byte 0");
	vAssertTrue(auEncoded[27] == 0x80U, "rasta sr confirmed timestamp byte 3");
	vAssertTrue(auEncoded[D_RSRX_CODEC_RASTA_SR_HEADER_BYTES] == 0xDEU, "rasta sr payload byte 0");
	vAssertTrue(auEncoded[D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + 2U] == 0x42U, "rasta sr payload byte 2");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_OK,
		"rasta sr no-checksum decode");
	vAssertTrue(xPacket.usPacketLength == xRequest.usPacketLength, "rasta sr decoded length");
	vAssertTrue(xPacket.usMessageType == (uint16_t)RSRX_RASTA_SR_TYPE_DATA, "rasta sr decoded type");
	vAssertTrue(xPacket.uReceiverId == 0x11223344U, "rasta sr decoded receiver");
	vAssertTrue(xPacket.uSenderId == 0x55667788U, "rasta sr decoded sender");
	vAssertTrue(xPacket.uSequenceNumber == 0x01020304U, "rasta sr decoded sequence");
	vAssertTrue(xPacket.uConfirmedSequenceNumber == 0x05060708U, "rasta sr decoded confirmed sequence");
	vAssertTrue(xPacket.uTimestamp == 0x10203040U, "rasta sr decoded timestamp");
	vAssertTrue(xPacket.uConfirmedTimestamp == 0x50607080U, "rasta sr decoded confirmed timestamp");
	vAssertTrue(xPacket.xPayloadLength == sizeof(auPayload), "rasta sr decoded payload length");
	vAssertTrue(xPacket.auPayload[1] == 0xADU, "rasta sr decoded payload");
	vAssertTrue(xPacket.xChecksumLength == 0U, "rasta sr decoded checksum length");
	vAssertTrue(xPacket.uChecksumPresent == 0U, "rasta sr decoded checksum absent");
}

static void vTestRastaSrNoChecksumEncodeRejectsInvalidInputs(void)
{
	static const uint8_t auPayload[1] = { 0x7EU };
	static const uint8_t auChecksum[1] = { 0xAAU };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES];
	rsrx_rasta_sr_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;

	xRequest.usPacketLength = (uint16_t)(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + sizeof(auPayload));
	xRequest.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xRequest.uReceiverId = 1U;
	xRequest.uSenderId = 2U;
	xRequest.uSequenceNumber = 3U;
	xRequest.uConfirmedSequenceNumber = 2U;
	xRequest.uTimestamp = 100U;
	xRequest.uConfirmedTimestamp = 90U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);
	xRequest.puChecksum = (const uint8_t *)0;
	xRequest.xChecksumLength = 0U;

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = D_RSRX_CODEC_RASTA_SR_HEADER_BYTES;
	xBuffer.xEncodedLength = 99U;

	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_BUFFER_TOO_SMALL,
		"rasta sr no-checksum small buffer reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "rasta sr no-checksum small buffer clears length");

	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 99U;
	xRequest.puChecksum = auChecksum;
	xRequest.xChecksumLength = sizeof(auChecksum);
	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr no-checksum supplied checksum reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "rasta sr no-checksum checksum reject clears length");

	xRequest.puChecksum = (const uint8_t *)0;
	xRequest.xChecksumLength = 0U;
	xRequest.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_RETRRESP;
	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		"rasta sr no-checksum unsupported type reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "rasta sr no-checksum unsupported type clears length");

	xRequest.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xRequest.usPacketLength = D_RSRX_CODEC_RASTA_SR_HEADER_BYTES;
	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_LENGTH_MISMATCH,
		"rasta sr no-checksum length mismatch reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "rasta sr no-checksum length mismatch clears length");

	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum((const rsrx_rasta_sr_encode_request_t *)0, &xBuffer) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr no-checksum null request reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "rasta sr no-checksum null request clears length");
	vAssertTrue(
		rsrx_codec_encode_rasta_sr_no_checksum(&xRequest, (rsrx_encode_buffer_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr no-checksum null buffer reject");
}

static void vTestRastaSrNoChecksumDecodeRejectsMalformedFrames(void)
{
	uint8_t auEncoded[D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + 1U] = { 0U };
	rsrx_transport_frame_t xFrame;
	rsrx_rasta_sr_decoded_packet_t xPacket;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x001DU, &auEncoded[0]) == RSRX_CODEC_STATUS_OK,
		"rasta sr fixture length");
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16((uint16_t)RSRX_RASTA_SR_TYPE_DATA, &auEncoded[2]) ==
			RSRX_CODEC_STATUS_OK,
		"rasta sr fixture type");

	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_OK,
		"rasta sr no-checksum baseline malformed fixture decode");
	vAssertTrue(xPacket.xPayloadLength == 1U, "rasta sr no-checksum baseline payload length");

	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, (rsrx_rasta_sr_decoded_packet_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr no-checksum null packet reject");

	xFrame.xPayloadLength = D_RSRX_CODEC_RASTA_SR_HEADER_BYTES - 1U;
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_SHORT_HEADER,
		"rasta sr no-checksum short header reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum short header clears packet");

	xFrame.xPayloadLength = sizeof(auEncoded);
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x001EU, &auEncoded[0]) == RSRX_CODEC_STATUS_OK,
		"rasta sr truncated fixture length");
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD,
		"rasta sr no-checksum truncated payload reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum truncated payload clears packet");

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x001CU, &auEncoded[0]) == RSRX_CODEC_STATUS_OK,
		"rasta sr trailing fixture length");
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_TRAILING_BYTES,
		"rasta sr no-checksum trailing bytes reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum trailing bytes clears packet");

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x0010U, &auEncoded[0]) == RSRX_CODEC_STATUS_OK,
		"rasta sr short declared fixture length");
	xFrame.xPayloadLength = D_RSRX_CODEC_RASTA_SR_HEADER_BYTES;
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_LENGTH_MISMATCH,
		"rasta sr no-checksum short declared length reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum short declared length clears packet");

	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16(0x001CU, &auEncoded[0]) == RSRX_CODEC_STATUS_OK,
		"rasta sr unsupported fixture length");
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16((uint16_t)RSRX_RASTA_SR_TYPE_RETRDATA, &auEncoded[2]) ==
			RSRX_CODEC_STATUS_OK,
		"rasta sr unsupported fixture type");
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		"rasta sr no-checksum unsupported type reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum unsupported type clears packet");

	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;
	vAssertTrue(
		rsrx_codec_write_rasta_sr_uint16((uint16_t)RSRX_RASTA_SR_TYPE_DATA, &auEncoded[2]) ==
			RSRX_CODEC_STATUS_OK,
		"rasta sr non-frame fixture type");
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_NON_FRAME_EVENT,
		"rasta sr no-checksum non-frame reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum non-frame clears packet");

	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	vSeedRastaSrDecodedPacket(&xPacket);
	vAssertTrue(
		rsrx_codec_decode_rasta_sr_no_checksum(&xFrame, &xPacket) == RSRX_CODEC_STATUS_INVALID_CHANNEL,
		"rasta sr no-checksum invalid channel reject");
	vAssertRastaSrDecodedPacketCleared(&xPacket, "rasta sr no-checksum invalid channel clears packet");
}

static void vTestRastaSrChecksumProfileAdmissionPolicy(void)
{
	rsrx_rasta_sr_checksum_profile_t xProfile;

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE;
	xProfile.xChecksumBytes = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) == RSRX_CODEC_STATUS_OK,
		"rasta sr checksum none profile accepted");

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_MD4;
	xProfile.xChecksumBytes = 8U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE,
		"rasta sr md4 checksum profile rejected");

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_BLAKE2B;
	xProfile.xChecksumBytes = 16U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE,
		"rasta sr blake2b checksum profile rejected");

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_SIPHASH_2_4;
	xProfile.xChecksumBytes = 8U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) ==
			RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE,
		"rasta sr siphash checksum profile rejected");

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE;
	xProfile.xChecksumBytes = 8U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr none with checksum bytes invalid");

	xProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_MD4;
	xProfile.xChecksumBytes = 4U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile(&xProfile) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr invalid checksum length rejected");

	vAssertTrue(
		rsrx_codec_validate_rasta_sr_checksum_profile((const rsrx_rasta_sr_checksum_profile_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr null checksum profile rejected");
}

static void vTestRastaSrTimestampAdmissionPolicy(void)
{
	rsrx_rasta_sr_decoded_packet_t xPacket;
	rsrx_rasta_sr_timestamp_admission_policy_t xPolicy;

	vSeedRastaSrDecodedPacket(&xPacket);
	xPacket.uTimestamp = 1000U;
	xPacket.uConfirmedTimestamp = 995U;
	xPolicy.uCurrentTimestamp = 1000U;
	xPolicy.uAcceptedPastWindow = 100U;
	xPolicy.uAcceptedFutureWindow = 10U;
	xPolicy.uLastAcceptedTimestamp = 900U;

	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) == RSRX_CODEC_STATUS_OK,
		"rasta sr timestamp admission accepted");

	xPacket.uTimestamp = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_ZERO,
		"rasta sr zero timestamp rejected");

	xPacket.uTimestamp = 1011U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE,
		"rasta sr future timestamp rejected");

	xPacket.uTimestamp = 899U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_REGRESSED,
		"rasta sr regressed timestamp rejected before stale classification");

	xPolicy.uLastAcceptedTimestamp = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_STALE,
		"rasta sr stale timestamp rejected");

	xPacket.uTimestamp = 1000U;
	xPacket.uConfirmedTimestamp = 1011U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE,
		"rasta sr future confirmed timestamp rejected");

	xPacket.uConfirmedTimestamp = 995U;
	xPolicy.uCurrentTimestamp = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_TIMESTAMP_ZERO,
		"rasta sr zero current timestamp rejected");

	xPolicy.uCurrentTimestamp = UINT32_MAX;
	xPolicy.uAcceptedFutureWindow = 1U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr timestamp future boundary overflow rejected");

	xPolicy.uCurrentTimestamp = 50U;
	xPolicy.uAcceptedFutureWindow = 0U;
	xPolicy.uAcceptedPastWindow = 51U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr timestamp past boundary underflow rejected");

	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(
			(const rsrx_rasta_sr_decoded_packet_t *)0,
			&xPolicy) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr timestamp null packet rejected");
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_timestamp_admission(
			&xPacket,
			(const rsrx_rasta_sr_timestamp_admission_policy_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr timestamp null policy rejected");
}

static void vTestRastaSrIdentityAdmissionPolicy(void)
{
	rsrx_rasta_sr_decoded_packet_t xPacket;
	rsrx_rasta_sr_identity_admission_policy_t xPolicy;

	vSeedRastaSrDecodedPacket(&xPacket);
	xPacket.uReceiverId = 0x11223344U;
	xPacket.uSenderId = 0x55667788U;
	xPolicy.uExpectedReceiverId = 0x11223344U;
	xPolicy.uExpectedSenderId = 0x55667788U;

	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(&xPacket, &xPolicy) == RSRX_CODEC_STATUS_OK,
		"rasta sr identity admission accepted");

	xPacket.uReceiverId = 0x11223345U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_RECEIVER_ID_MISMATCH,
		"rasta sr receiver mismatch rejected");

	xPacket.uReceiverId = 0x11223344U;
	xPacket.uSenderId = 0x55667789U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_SENDER_ID_MISMATCH,
		"rasta sr sender mismatch rejected");

	xPacket.uSenderId = 0x55667788U;
	xPolicy.uExpectedReceiverId = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr zero expected receiver rejected");

	xPolicy.uExpectedReceiverId = 0x11223344U;
	xPolicy.uExpectedSenderId = 0U;
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(&xPacket, &xPolicy) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr zero expected sender rejected");

	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(
			(const rsrx_rasta_sr_decoded_packet_t *)0,
			&xPolicy) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr identity null packet rejected");
	vAssertTrue(
		rsrx_codec_validate_rasta_sr_identity_admission(
			&xPacket,
			(const rsrx_rasta_sr_identity_admission_policy_t *)0) ==
			RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr identity null policy rejected");
}

static void vTestRastaSrTimestampAdmittedSessionHandoffMapping(void)
{
	rsrx_rasta_sr_decoded_packet_t xPacket;
	rsrx_rasta_sr_timestamp_admission_policy_t xPolicy;
	rsrx_decoded_message_t xMessage;

	vSeedRastaSrDecodedPacket(&xPacket);
	xPacket.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xPacket.uSequenceNumber = 17U;
	xPacket.uConfirmedSequenceNumber = 16U;
	xPacket.uTimestamp = 1000U;
	xPacket.uConfirmedTimestamp = 995U;
	xPacket.xPayloadLength = 2U;
	xPacket.auPayload[0] = 0xA1U;
	xPacket.auPayload[1] = 0xB2U;
	xPolicy.uCurrentTimestamp = 1000U;
	xPolicy.uAcceptedPastWindow = 100U;
	xPolicy.uAcceptedFutureWindow = 10U;
	xPolicy.uLastAcceptedTimestamp = 900U;

	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			&xPacket,
			&xPolicy,
			&xMessage) == RSRX_CODEC_STATUS_OK,
		"rasta sr timestamp admitted handoff mapping");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_DATA, "rasta sr handoff message type");
	vAssertTrue(xMessage.eSuggestedEvent == RSRX_EVENT_VALID_DATA, "rasta sr handoff event");
	vAssertTrue(xMessage.eReason == RSRX_REASON_DATA_ACCEPTED, "rasta sr handoff reason");
	vAssertTrue(xMessage.uSequenceNumber == 17U, "rasta sr handoff sequence");
	vAssertTrue(xMessage.uConfirmationNumber == 16U, "rasta sr handoff confirmation");
	vAssertTrue(xMessage.xPayloadLength == 2U, "rasta sr handoff payload length");
	vAssertTrue(xMessage.auPayload[0] == 0xA1U, "rasta sr handoff payload byte 0");
	vAssertTrue(xMessage.auPayload[1] == 0xB2U, "rasta sr handoff payload byte 1");
}

static void vTestRastaSrIdentityAndTimestampAdmittedSessionHandoffMapping(void)
{
	rsrx_rasta_sr_decoded_packet_t xPacket;
	rsrx_rasta_sr_timestamp_admission_policy_t xTimestampPolicy;
	rsrx_rasta_sr_identity_admission_policy_t xIdentityPolicy;
	rsrx_decoded_message_t xMessage;

	vSeedRastaSrDecodedPacket(&xPacket);
	xPacket.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xPacket.uReceiverId = 0x11223344U;
	xPacket.uSenderId = 0x55667788U;
	xPacket.uSequenceNumber = 19U;
	xPacket.uConfirmedSequenceNumber = 18U;
	xPacket.uTimestamp = 1000U;
	xPacket.uConfirmedTimestamp = 995U;
	xPacket.xPayloadLength = 1U;
	xPacket.auPayload[0] = 0xC3U;
	xTimestampPolicy.uCurrentTimestamp = 1000U;
	xTimestampPolicy.uAcceptedPastWindow = 100U;
	xTimestampPolicy.uAcceptedFutureWindow = 10U;
	xTimestampPolicy.uLastAcceptedTimestamp = 900U;
	xIdentityPolicy.uExpectedReceiverId = 0x11223344U;
	xIdentityPolicy.uExpectedSenderId = 0x55667788U;

	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			&xIdentityPolicy,
			&xMessage) == RSRX_CODEC_STATUS_OK,
		"rasta sr identity timestamp admitted handoff mapping");
	vAssertTrue(xMessage.eMessageType == RSRX_MESSAGE_TYPE_DATA, "rasta sr identity handoff message type");
	vAssertTrue(xMessage.uSequenceNumber == 19U, "rasta sr identity handoff sequence");
	vAssertTrue(xMessage.uConfirmationNumber == 18U, "rasta sr identity handoff confirmation");
	vAssertTrue(xMessage.xPayloadLength == 1U, "rasta sr identity handoff payload length");
	vAssertTrue(xMessage.auPayload[0] == 0xC3U, "rasta sr identity handoff payload byte");

	xPacket.uReceiverId = 0x11223345U;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			&xIdentityPolicy,
			&xMessage) == RSRX_CODEC_STATUS_RECEIVER_ID_MISMATCH,
		"rasta sr identity handoff receiver mismatch rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr identity handoff receiver mismatch clears message");

	xPacket.uReceiverId = 0x11223344U;
	xPacket.uSenderId = 0x55667789U;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			&xIdentityPolicy,
			&xMessage) == RSRX_CODEC_STATUS_SENDER_ID_MISMATCH,
		"rasta sr identity handoff sender mismatch rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr identity handoff sender mismatch clears message");

	xPacket.uSenderId = 0x55667788U;
	xPacket.uTimestamp = 1200U;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			&xIdentityPolicy,
			&xMessage) == RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE,
		"rasta sr identity handoff future timestamp rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr identity handoff future timestamp clears message");

	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			(const rsrx_rasta_sr_identity_admission_policy_t *)0,
			&xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr identity handoff null identity policy rejected");
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
			&xPacket,
			&xTimestampPolicy,
			&xIdentityPolicy,
			(rsrx_decoded_message_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr identity handoff null message rejected");
}

static void vTestRastaSrTimestampHandoffRejectsBeforeMessageMapping(void)
{
	rsrx_rasta_sr_decoded_packet_t xPacket;
	rsrx_rasta_sr_timestamp_admission_policy_t xPolicy;
	rsrx_decoded_message_t xMessage;

	vSeedRastaSrDecodedPacket(&xPacket);
	xPacket.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_DATA;
	xPacket.uTimestamp = 1200U;
	xPacket.uConfirmedTimestamp = 1000U;
	xPolicy.uCurrentTimestamp = 1000U;
	xPolicy.uAcceptedPastWindow = 100U;
	xPolicy.uAcceptedFutureWindow = 10U;
	xPolicy.uLastAcceptedTimestamp = 900U;
	vSeedDecodedMessage(&xMessage);

	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			&xPacket,
			&xPolicy,
			&xMessage) == RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE,
		"rasta sr handoff future timestamp rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr handoff future timestamp clears message");

	xPacket.uTimestamp = 1000U;
	xPacket.usMessageType = (uint16_t)RSRX_RASTA_SR_TYPE_RETRDATA;
	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			&xPacket,
			&xPolicy,
			&xMessage) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
		"rasta sr handoff unsupported type rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr handoff unsupported type clears message");

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			(const rsrx_rasta_sr_decoded_packet_t *)0,
			&xPolicy,
			&xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr handoff null packet rejected");
	vAssertDecodedMessageCleared(&xMessage, "rasta sr handoff null packet clears message");
	vAssertTrue(
		rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
			&xPacket,
			&xPolicy,
			(rsrx_decoded_message_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"rasta sr handoff null message rejected");
}

static void vTestCrc32InjectedCalculatorPortability(void)
{
	uint8_t auPayload[2] = { 0x5AU, 0xC3U };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 23U;
	xRequest.uConfirmationNumber = 22U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	uInjectedCrcCallCount = 0U;
	vAssertTrue(
		rsrx_codec_encode_message_with_crc32_calculator(&xRequest, &xBuffer, eInjectedCrc32Calculator) ==
			RSRX_CODEC_STATUS_OK,
		"injected crc32 encode");
	vAssertTrue(uInjectedCrcCallCount == 1U, "injected crc32 encode calculator called");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32_calculator(&xFrame, &xMessage, eInjectedCrc32Calculator) ==
			RSRX_CODEC_STATUS_OK,
		"injected crc32 decode");
	vAssertTrue(uInjectedCrcCallCount == 2U, "injected crc32 decode calculator called");
	vAssertTrue(xMessage.uSequenceNumber == 23U, "injected crc32 decoded sequence");
	vAssertTrue(xMessage.auPayload[1] == 0xC3U, "injected crc32 decoded payload");

	xBuffer.xEncodedLength = 99U;
	uInjectedCrcCallCount = 0U;
	vAssertTrue(
		rsrx_codec_encode_message_with_crc32_calculator(&xRequest, &xBuffer, eFailingInjectedCrc32Calculator) ==
			RSRX_CODEC_STATUS_DECODE_ERROR,
		"injected crc32 encode failure status");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "injected crc32 encode failure clears length");
	vAssertTrue(uInjectedCrcCallCount == 1U, "injected crc32 encode failure calculator called");

	vSeedDecodedMessage(&xMessage);
	uInjectedCrcCallCount = 0U;
	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32_calculator(&xFrame, &xMessage, eFailingInjectedCrc32Calculator) ==
			RSRX_CODEC_STATUS_DECODE_ERROR,
		"injected crc32 decode failure status");
	vAssertDecodedMessageCleared(&xMessage, "injected crc32 decode failure clears stale decoded message");
	vAssertTrue(uInjectedCrcCallCount == 1U, "injected crc32 decode failure calculator called");

	xBuffer.xEncodedLength = 99U;
	vAssertTrue(
		rsrx_codec_encode_message_with_crc32_calculator(
			&xRequest,
			&xBuffer,
			(rsrx_crc32_calculate_fn)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"injected crc32 null calculator encode reject");
	vAssertTrue(xBuffer.xEncodedLength == 0U, "injected crc32 null calculator encode clears length");

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32_calculator(
			&xFrame,
			&xMessage,
			(rsrx_crc32_calculate_fn)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT,
		"injected crc32 null calculator decode reject");
	vAssertDecodedMessageCleared(&xMessage, "injected crc32 null calculator decode clears stale decoded message");
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

static void vTestCrc32WirePreservesTransportMetadataDecodeStatus(void)
{
	uint8_t auPayload[1] = { 0x61U };
	uint8_t auEncoded[D_RSRX_CODEC_MAX_CRC_FRAME_BYTES];
	rsrx_encode_request_t xRequest;
	rsrx_encode_buffer_t xBuffer;
	rsrx_transport_frame_t xFrame;
	rsrx_decoded_message_t xMessage;

	xRequest.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xRequest.eReason = RSRX_REASON_DATA_ACCEPTED;
	xRequest.uSequenceNumber = 31U;
	xRequest.uConfirmationNumber = 30U;
	xRequest.puPayload = auPayload;
	xRequest.xPayloadLength = sizeof(auPayload);

	xBuffer.puBuffer = auEncoded;
	xBuffer.xBufferCapacity = sizeof(auEncoded);
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(
		rsrx_codec_encode_message_with_crc32(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_OK,
		"crc32 metadata encode");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = xBuffer.xEncodedLength;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_SEND_COMPLETED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) ==
			RSRX_CODEC_STATUS_NON_FRAME_EVENT,
		"crc32 wrapped non-frame event status");
	vAssertDecodedMessageCleared(&xMessage, "crc32 wrapped non-frame clears stale decoded message");

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vSeedDecodedMessage(&xMessage);
	vAssertTrue(
		rsrx_codec_decode_frame_with_crc32(&xFrame, &xMessage) ==
			RSRX_CODEC_STATUS_INVALID_CHANNEL,
		"crc32 wrapped invalid channel status");
	vAssertDecodedMessageCleared(&xMessage, "crc32 wrapped invalid channel clears stale decoded message");
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
	vTestRastaSrWireProfileDocumentsParityTarget();
	vTestRastaSrMessageTypeMapping();
	vTestRastaDisconnectReasonMapping();
	vTestRastaSrByteOrderIsFixedBigEndian();
	vTestRastaSrNoChecksumEncodeDecodeRoundTrip();
	vTestRastaSrNoChecksumEncodeRejectsInvalidInputs();
	vTestRastaSrNoChecksumDecodeRejectsMalformedFrames();
	vTestRastaSrChecksumProfileAdmissionPolicy();
	vTestRastaSrTimestampAdmissionPolicy();
	vTestRastaSrIdentityAdmissionPolicy();
	vTestRastaSrTimestampAdmittedSessionHandoffMapping();
	vTestRastaSrIdentityAndTimestampAdmittedSessionHandoffMapping();
	vTestRastaSrTimestampHandoffRejectsBeforeMessageMapping();
	vTestCrc32InjectedCalculatorPortability();
	vTestCrc32PrimitiveKnownVector();
	vTestCrc32PrimitiveRejectsInvalidArguments();
	vTestCrc32WireRoundTrip();
	vTestCrc32WireRejectsSmallBuffer();
	vTestCrc32WireReportsChecksumMismatch();
	vTestCrc32WireReportsTruncatedChecksum();
	vTestCrc32WireRejectsNullDecodeArguments();
	vTestCrc32WirePreservesPayloadDecodeStatus();
	vTestCrc32WirePreservesTransportMetadataDecodeStatus();
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
