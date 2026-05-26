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

static rsrx_codec_status_t eContractCrc32Calculator(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc)
{
	(void)puData;
	(void)xDataLength;
	if(puCrc == (uint32_t *)0)
	{
		return RSRX_CODEC_STATUS_INVALID_ARGUMENT;
	}

	*puCrc = 0U;
	return RSRX_CODEC_STATUS_OK;
}

int main(void)
{
	uint8_t auPayload[4] = { 0x11U, 0x22U, 0x33U, 0x44U };
	uint8_t auBuffer[32] = { 0U };
	uint32_t uExpectedEncodedLength;
	volatile uint32_t uExpectedAbsent;
	volatile size_t xExpectedCrcCalculatorTypeSize;
	rsrx_crc32_calculate_fn pfExpectedCrc32Calculator;
	rsrx_decode_frame_fn pfExpectedDecode;
	rsrx_decoded_message_t xDecodedMessage;
	rsrx_encode_request_t xEncodeRequest;
	rsrx_encode_buffer_t xEncodeBuffer;
	rsrx_codec_port_t xCodecPort;
	rsrx_codec_wire_profile_t xWireProfile;
	rsrx_rasta_sr_encode_request_t xRastaSrEncodeRequest;
	rsrx_rasta_sr_decoded_packet_t xRastaSrDecodedPacket;

	xDecodedMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xDecodedMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xDecodedMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xDecodedMessage.uSequenceNumber = 7U;
	xDecodedMessage.uConfirmationNumber = 6U;
	xDecodedMessage.xPayloadLength = sizeof(auPayload);

	xEncodeRequest.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_REQUEST;
	xEncodeRequest.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xEncodeRequest.uSequenceNumber = 1U;
	xEncodeRequest.uConfirmationNumber = 0U;
	xEncodeRequest.puPayload = auPayload;
	xEncodeRequest.xPayloadLength = sizeof(auPayload);

	xEncodeBuffer.puBuffer = auBuffer;
	xEncodeBuffer.xBufferCapacity = sizeof(auBuffer);
	uExpectedEncodedLength = 0U;
	uExpectedAbsent = 0U;
	xExpectedCrcCalculatorTypeSize = sizeof(pfExpectedCrc32Calculator);
	xEncodeBuffer.xEncodedLength = uExpectedEncodedLength;

	xCodecPort.pfEncode = (rsrx_encode_message_fn)0;
	pfExpectedCrc32Calculator = eContractCrc32Calculator;
	pfExpectedDecode = (rsrx_decode_frame_fn)0;
	xCodecPort.pfDecode = pfExpectedDecode;
	xWireProfile.uProfileId = D_RSRX_CODEC_WIRE_PROFILE_DEFAULT;
	xWireProfile.uProfileVersion = D_RSRX_CODEC_WIRE_PROFILE_VERSION;
	xWireProfile.xHeaderBytes = D_RSRX_CODEC_HEADER_BYTES;
	xWireProfile.xMaxPayloadBytes = D_RSRX_CODEC_MAX_PAYLOAD_BYTES;
	xWireProfile.xMaxFrameBytes = D_RSRX_CODEC_MAX_FRAME_BYTES;
	xWireProfile.xCrcBytes = (size_t)uExpectedAbsent;
	xWireProfile.xMacBytes = (size_t)uExpectedAbsent;
	xWireProfile.xTimestampBytes = (size_t)uExpectedAbsent;
	xWireProfile.uCrcPresent = uExpectedAbsent;
	xWireProfile.uMacPresent = uExpectedAbsent;
	xWireProfile.uTimestampPresent = uExpectedAbsent;

	xRastaSrEncodeRequest.usPacketLength = (uint16_t)(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + sizeof(auPayload));
	xRastaSrEncodeRequest.usMessageType = 6240U;
	xRastaSrEncodeRequest.uReceiverId = 0x11223344U;
	xRastaSrEncodeRequest.uSenderId = 0x55667788U;
	xRastaSrEncodeRequest.uSequenceNumber = 9U;
	xRastaSrEncodeRequest.uConfirmedSequenceNumber = 8U;
	xRastaSrEncodeRequest.uTimestamp = 7000U;
	xRastaSrEncodeRequest.uConfirmedTimestamp = 6000U;
	xRastaSrEncodeRequest.puPayload = auPayload;
	xRastaSrEncodeRequest.xPayloadLength = sizeof(auPayload);
	xRastaSrEncodeRequest.puChecksum = (const uint8_t *)0;
	xRastaSrEncodeRequest.xChecksumLength = 0U;

	xRastaSrDecodedPacket.usPacketLength = xRastaSrEncodeRequest.usPacketLength;
	xRastaSrDecodedPacket.usMessageType = xRastaSrEncodeRequest.usMessageType;
	xRastaSrDecodedPacket.uReceiverId = xRastaSrEncodeRequest.uReceiverId;
	xRastaSrDecodedPacket.uSenderId = xRastaSrEncodeRequest.uSenderId;
	xRastaSrDecodedPacket.uSequenceNumber = xRastaSrEncodeRequest.uSequenceNumber;
	xRastaSrDecodedPacket.uConfirmedSequenceNumber = xRastaSrEncodeRequest.uConfirmedSequenceNumber;
	xRastaSrDecodedPacket.uTimestamp = xRastaSrEncodeRequest.uTimestamp;
	xRastaSrDecodedPacket.uConfirmedTimestamp = xRastaSrEncodeRequest.uConfirmedTimestamp;
	xRastaSrDecodedPacket.xPayloadLength = sizeof(auPayload);
	xRastaSrDecodedPacket.auPayload[0] = auPayload[0];
	xRastaSrDecodedPacket.xChecksumLength = 0U;
	xRastaSrDecodedPacket.uChecksumPresent = 0U;

	vAssertTrue(xDecodedMessage.eMessageType == RSRX_MESSAGE_TYPE_DATA, "decoded message type contract");
	vAssertTrue(xDecodedMessage.eSuggestedEvent == RSRX_EVENT_VALID_DATA, "decoded message event contract");
	vAssertTrue(xDecodedMessage.eReason == RSRX_REASON_DATA_ACCEPTED, "decoded message reason contract");
	vAssertTrue(xDecodedMessage.uSequenceNumber == 7U, "decoded message sequence contract");
	vAssertTrue(xDecodedMessage.uConfirmationNumber == 6U, "decoded message confirmation contract");
	vAssertTrue(xDecodedMessage.xPayloadLength == sizeof(auPayload), "decoded message payload contract");
	vAssertTrue(xEncodeRequest.eMessageType == RSRX_MESSAGE_TYPE_CONNECT_REQUEST, "encode request type contract");
	vAssertTrue(xEncodeRequest.eReason == RSRX_REASON_CONNECT_REQUESTED, "encode request reason contract");
	vAssertTrue(xEncodeRequest.uSequenceNumber == 1U, "encode request sequence contract");
	vAssertTrue(xEncodeRequest.uConfirmationNumber == 0U, "encode request confirmation contract");
	vAssertTrue(xEncodeRequest.puPayload == auPayload, "encode request payload pointer contract");
	vAssertTrue(xEncodeRequest.xPayloadLength == sizeof(auPayload), "encode request payload length contract");
	vAssertTrue(xEncodeBuffer.puBuffer == auBuffer, "encode buffer pointer contract");
	vAssertTrue(xEncodeBuffer.xBufferCapacity == sizeof(auBuffer), "encode buffer contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xEncodeBuffer.xEncodedLength == uExpectedEncodedLength, "encode buffer encoded length contract");
	vAssertTrue(xCodecPort.pfEncode == (rsrx_encode_message_fn)0, "codec port layout contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xCodecPort.pfDecode == pfExpectedDecode, "codec port decode layout contract");
	vAssertTrue(pfExpectedCrc32Calculator == eContractCrc32Calculator, "crc32 calculator type contract");
	vAssertTrue(xExpectedCrcCalculatorTypeSize == sizeof(pfExpectedCrc32Calculator), "crc32 calculator size contract");
	vAssertTrue(xWireProfile.uProfileId == D_RSRX_CODEC_WIRE_PROFILE_DEFAULT, "wire profile id contract");
	vAssertTrue(xWireProfile.uProfileVersion == D_RSRX_CODEC_WIRE_PROFILE_VERSION, "wire profile version contract");
	vAssertTrue(xWireProfile.xHeaderBytes == D_RSRX_CODEC_HEADER_BYTES, "wire profile header contract");
	vAssertTrue(xWireProfile.xMaxPayloadBytes == D_RSRX_CODEC_MAX_PAYLOAD_BYTES, "wire profile max payload contract");
	vAssertTrue(xWireProfile.xMaxFrameBytes == D_RSRX_CODEC_MAX_FRAME_BYTES, "wire profile max frame contract");
	vAssertTrue(xWireProfile.xCrcBytes == (size_t)uExpectedAbsent, "wire profile crc bytes contract");
	vAssertTrue(xWireProfile.xMacBytes == (size_t)uExpectedAbsent, "wire profile mac bytes contract");
	vAssertTrue(xWireProfile.xTimestampBytes == (size_t)uExpectedAbsent, "wire profile timestamp bytes contract");
	vAssertTrue(xWireProfile.uCrcPresent == uExpectedAbsent, "wire profile crc present contract");
	vAssertTrue(xWireProfile.uMacPresent == uExpectedAbsent, "wire profile mac present contract");
	vAssertTrue(xWireProfile.uTimestampPresent == uExpectedAbsent, "wire profile timestamp present contract");
	vAssertTrue(D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR != D_RSRX_CODEC_WIRE_PROFILE_DEFAULT, "rasta sr profile id differs from default");
	vAssertTrue(D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR != D_RSRX_CODEC_WIRE_PROFILE_CRC32, "rasta sr profile id differs from crc32");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES == 28U, "rasta sr header bytes contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_TIMESTAMP_BYTES == 8U, "rasta sr timestamp bytes contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_MAX_CHECKSUM_BYTES == 16U, "rasta sr max checksum bytes contract");
	vAssertTrue(D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES == (D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + D_RSRX_CODEC_MAX_PAYLOAD_BYTES), "rasta sr max frame contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_CONNREQ == 6200U, "rasta sr connreq numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_CONNRESP == 6201U, "rasta sr connresp numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_RETRREQ == 6212U, "rasta sr retrreq numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_RETRRESP == 6213U, "rasta sr retrresp numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_DISCREQ == 6216U, "rasta sr discreq numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_HB == 6220U, "rasta sr hb numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_DATA == 6240U, "rasta sr data numeric contract");
	vAssertTrue(RSRX_RASTA_SR_TYPE_RETRDATA == 6241U, "rasta sr retrdata numeric contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_USERREQUEST == 0U, "rasta disc user request contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_UNEXPECTEDTYPE == 2U, "rasta disc unexpected type contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_SEQNERROR == 3U, "rasta disc seq error contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_TIMEOUT == 4U, "rasta disc timeout contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_SERVICENOTALLOWED == 5U, "rasta disc service not allowed contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_INCOMPATIBLEVERSION == 6U, "rasta disc incompatible version contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_RETRFAILED == 7U, "rasta disc retransmission failed contract");
	vAssertTrue(RSRX_RASTA_DISC_REASON_PROTOCOLERROR == 8U, "rasta disc protocol error contract");
	vAssertTrue(xRastaSrEncodeRequest.usPacketLength == (uint16_t)(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + sizeof(auPayload)), "rasta sr encode length contract");
	vAssertTrue(xRastaSrEncodeRequest.usMessageType == 6240U, "rasta sr encode type contract");
	vAssertTrue(xRastaSrEncodeRequest.uReceiverId == 0x11223344U, "rasta sr encode receiver contract");
	vAssertTrue(xRastaSrEncodeRequest.uSenderId == 0x55667788U, "rasta sr encode sender contract");
	vAssertTrue(xRastaSrEncodeRequest.uSequenceNumber == 9U, "rasta sr encode sequence contract");
	vAssertTrue(xRastaSrEncodeRequest.uConfirmedSequenceNumber == 8U, "rasta sr encode confirmed sequence contract");
	vAssertTrue(xRastaSrEncodeRequest.uTimestamp == 7000U, "rasta sr encode timestamp contract");
	vAssertTrue(xRastaSrEncodeRequest.uConfirmedTimestamp == 6000U, "rasta sr encode confirmed timestamp contract");
	vAssertTrue(xRastaSrEncodeRequest.puPayload == auPayload, "rasta sr encode payload pointer contract");
	vAssertTrue(xRastaSrEncodeRequest.xPayloadLength == sizeof(auPayload), "rasta sr encode payload length contract");
	vAssertTrue(xRastaSrEncodeRequest.puChecksum == (const uint8_t *)0, "rasta sr encode checksum pointer contract");
	vAssertTrue(xRastaSrEncodeRequest.xChecksumLength == (size_t)uExpectedAbsent, "rasta sr encode checksum length contract");
	vAssertTrue(xRastaSrDecodedPacket.usPacketLength == xRastaSrEncodeRequest.usPacketLength, "rasta sr decoded length contract");
	vAssertTrue(xRastaSrDecodedPacket.usMessageType == xRastaSrEncodeRequest.usMessageType, "rasta sr decoded type contract");
	vAssertTrue(xRastaSrDecodedPacket.uReceiverId == xRastaSrEncodeRequest.uReceiverId, "rasta sr decoded receiver contract");
	vAssertTrue(xRastaSrDecodedPacket.uSenderId == xRastaSrEncodeRequest.uSenderId, "rasta sr decoded sender contract");
	vAssertTrue(xRastaSrDecodedPacket.uSequenceNumber == xRastaSrEncodeRequest.uSequenceNumber, "rasta sr decoded sequence contract");
	vAssertTrue(xRastaSrDecodedPacket.uConfirmedSequenceNumber == xRastaSrEncodeRequest.uConfirmedSequenceNumber, "rasta sr decoded confirmed sequence contract");
	vAssertTrue(xRastaSrDecodedPacket.uTimestamp == xRastaSrEncodeRequest.uTimestamp, "rasta sr decoded timestamp contract");
	vAssertTrue(xRastaSrDecodedPacket.uConfirmedTimestamp == xRastaSrEncodeRequest.uConfirmedTimestamp, "rasta sr decoded confirmed timestamp contract");
	vAssertTrue(xRastaSrDecodedPacket.xPayloadLength == sizeof(auPayload), "rasta sr decoded payload length contract");
	vAssertTrue(xRastaSrDecodedPacket.auPayload[0] == auPayload[0], "rasta sr decoded payload storage contract");
	vAssertTrue(xRastaSrDecodedPacket.xChecksumLength == 0U, "rasta sr decoded checksum length contract");
	vAssertTrue(xRastaSrDecodedPacket.uChecksumPresent == uExpectedAbsent, "rasta sr decoded checksum present contract");

	(void)printf("rsrx_codec_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
