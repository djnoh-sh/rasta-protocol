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
	volatile uint32_t uExpectedLastAcceptedTimestamp;
	volatile uint32_t uExpectedNoRedundancyCrcPresent;
	volatile size_t xExpectedNoChecksumBytes;
	volatile size_t xExpectedNoRedundancyCrcBytes;
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
	rsrx_rasta_redundancy_encode_request_t xRastaRedundancyEncodeRequest;
	rsrx_rasta_redundancy_decoded_packet_t xRastaRedundancyDecodedPacket;
	rsrx_rasta_sr_checksum_profile_t xRastaSrChecksumProfile;
	rsrx_rasta_redundancy_crc_profile_t xRastaRedundancyCrcProfile;
	const rsrx_rasta_redundancy_crc_profile_t * volatile pxExpectedRedundancyCrcProfile;
	rsrx_rasta_sr_timestamp_admission_policy_t xRastaSrTimestampPolicy;
	rsrx_rasta_sr_identity_admission_policy_t xRastaSrIdentityPolicy;
	volatile size_t xExpectedRastaSrHandoffPointerSize;
	rsrx_codec_status_t (* volatile pfRastaSrHandoff)(
		const rsrx_rasta_sr_decoded_packet_t *,
		const rsrx_rasta_sr_timestamp_admission_policy_t *,
		rsrx_decoded_message_t *);
	volatile size_t xExpectedRastaSrIdentityHandoffPointerSize;
	rsrx_codec_status_t (* volatile pfRastaSrIdentityHandoff)(
		const rsrx_rasta_sr_decoded_packet_t *,
		const rsrx_rasta_sr_timestamp_admission_policy_t *,
		const rsrx_rasta_sr_identity_admission_policy_t *,
		rsrx_decoded_message_t *);

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
	uExpectedLastAcceptedTimestamp = 900U;
	uExpectedNoRedundancyCrcPresent = 0U;
	xExpectedNoChecksumBytes = 0U;
	xExpectedNoRedundancyCrcBytes = 0U;
	xExpectedCrcCalculatorTypeSize = sizeof(pfExpectedCrc32Calculator);
	xExpectedRastaSrHandoffPointerSize = sizeof(pfRastaSrHandoff);
	xExpectedRastaSrIdentityHandoffPointerSize = sizeof(pfRastaSrIdentityHandoff);
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
	xRastaSrChecksumProfile.eAlgorithm = RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE;
	xRastaSrChecksumProfile.xChecksumBytes = xExpectedNoChecksumBytes;
	xRastaRedundancyCrcProfile.eOption = RSRX_RASTA_REDUNDANCY_CRC_OPTION_A;
	xRastaRedundancyCrcProfile.xCrcBytes = xExpectedNoRedundancyCrcBytes;
	pxExpectedRedundancyCrcProfile = &xRastaRedundancyCrcProfile;
	xRastaRedundancyEncodeRequest.usPacketLength =
		(uint16_t)(D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES +
			D_RSRX_CODEC_RASTA_SR_HEADER_BYTES);
	xRastaRedundancyEncodeRequest.usReserve = 0U;
	xRastaRedundancyEncodeRequest.uSequenceNumber = 15U;
	xRastaRedundancyEncodeRequest.puCarriedPacket = auPayload;
	xRastaRedundancyEncodeRequest.xCarriedPacketLength = D_RSRX_CODEC_RASTA_SR_HEADER_BYTES;
	xRastaRedundancyEncodeRequest.pxCrcProfile = pxExpectedRedundancyCrcProfile;
	xRastaRedundancyDecodedPacket.usPacketLength =
		xRastaRedundancyEncodeRequest.usPacketLength;
	xRastaRedundancyDecodedPacket.usReserve = xRastaRedundancyEncodeRequest.usReserve;
	xRastaRedundancyDecodedPacket.uSequenceNumber =
		xRastaRedundancyEncodeRequest.uSequenceNumber;
	xRastaRedundancyDecodedPacket.xCarriedPacketLength =
		xRastaRedundancyEncodeRequest.xCarriedPacketLength;
	xRastaRedundancyDecodedPacket.auCarriedPacket[0] = auPayload[0];
	xRastaRedundancyDecodedPacket.xCrcLength = xExpectedNoRedundancyCrcBytes;
	xRastaRedundancyDecodedPacket.uCrcPresent = uExpectedNoRedundancyCrcPresent;
	xRastaSrTimestampPolicy.uCurrentTimestamp = 1000U;
	xRastaSrTimestampPolicy.uAcceptedPastWindow = 100U;
	xRastaSrTimestampPolicy.uAcceptedFutureWindow = 10U;
	xRastaSrTimestampPolicy.uLastAcceptedTimestamp = uExpectedLastAcceptedTimestamp;
	xRastaSrIdentityPolicy.uExpectedReceiverId = xRastaSrEncodeRequest.uReceiverId;
	xRastaSrIdentityPolicy.uExpectedSenderId = xRastaSrEncodeRequest.uSenderId;
	pfRastaSrHandoff = (rsrx_codec_status_t (*)(
		const rsrx_rasta_sr_decoded_packet_t *,
		const rsrx_rasta_sr_timestamp_admission_policy_t *,
		rsrx_decoded_message_t *))0;
	pfRastaSrIdentityHandoff = (rsrx_codec_status_t (*)(
		const rsrx_rasta_sr_decoded_packet_t *,
		const rsrx_rasta_sr_timestamp_admission_policy_t *,
		const rsrx_rasta_sr_identity_admission_policy_t *,
		rsrx_decoded_message_t *))0;

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
	vAssertTrue(D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY != D_RSRX_CODEC_WIRE_PROFILE_DEFAULT,
		"rasta redundancy profile id differs from default");
	vAssertTrue(D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY != D_RSRX_CODEC_WIRE_PROFILE_CRC32,
		"rasta redundancy profile id differs from crc32");
	vAssertTrue(D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY != D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR,
		"rasta redundancy profile id differs from rasta sr");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_HEADER_BYTES == 28U, "rasta sr header bytes contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_TIMESTAMP_BYTES == 8U, "rasta sr timestamp bytes contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_MAX_CHECKSUM_BYTES == 16U, "rasta sr max checksum bytes contract");
	vAssertTrue(D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES == (D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + D_RSRX_CODEC_MAX_PAYLOAD_BYTES), "rasta sr max frame contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES == 8U,
		"rasta redundancy header bytes contract");
	vAssertTrue(D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES == 4U,
		"rasta redundancy max crc bytes contract");
	vAssertTrue(D_RSRX_CODEC_MAX_RASTA_REDUNDANCY_FRAME_BYTES ==
			(D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES +
				D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES +
				D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES),
		"rasta redundancy max frame contract");
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
	vAssertTrue(D_RSRX_CODEC_RASTA_SR_BYTE_ORDER_BIG_ENDIAN == 1U, "rasta sr big-endian byte order contract");
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
	vAssertTrue(RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE == 0, "rasta sr checksum none contract");
	vAssertTrue(RSRX_RASTA_SR_CHECKSUM_ALGORITHM_MD4 != RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE, "rasta sr checksum md4 contract");
	vAssertTrue(RSRX_RASTA_SR_CHECKSUM_ALGORITHM_BLAKE2B != RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE, "rasta sr checksum blake2b contract");
	vAssertTrue(RSRX_RASTA_SR_CHECKSUM_ALGORITHM_SIPHASH_2_4 != RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE, "rasta sr checksum siphash contract");
	vAssertTrue(xRastaSrChecksumProfile.eAlgorithm == RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE, "rasta sr checksum algorithm field contract");
	vAssertTrue(xRastaSrChecksumProfile.xChecksumBytes == xExpectedNoChecksumBytes, "rasta sr checksum bytes field contract");
	vAssertTrue(RSRX_RASTA_REDUNDANCY_CRC_OPTION_A == 0, "rasta redundancy crc option a contract");
	vAssertTrue(RSRX_RASTA_REDUNDANCY_CRC_OPTION_B != RSRX_RASTA_REDUNDANCY_CRC_OPTION_A,
		"rasta redundancy crc option b contract");
	vAssertTrue(RSRX_RASTA_REDUNDANCY_CRC_OPTION_C != RSRX_RASTA_REDUNDANCY_CRC_OPTION_A,
		"rasta redundancy crc option c contract");
	vAssertTrue(RSRX_RASTA_REDUNDANCY_CRC_OPTION_D != RSRX_RASTA_REDUNDANCY_CRC_OPTION_A,
		"rasta redundancy crc option d contract");
	vAssertTrue(RSRX_RASTA_REDUNDANCY_CRC_OPTION_E != RSRX_RASTA_REDUNDANCY_CRC_OPTION_A,
		"rasta redundancy crc option e contract");
	vAssertTrue(xRastaRedundancyCrcProfile.eOption == RSRX_RASTA_REDUNDANCY_CRC_OPTION_A,
		"rasta redundancy crc option field contract");
	vAssertTrue(xRastaRedundancyCrcProfile.xCrcBytes == xExpectedNoRedundancyCrcBytes,
		"rasta redundancy crc bytes field contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.usPacketLength ==
			(uint16_t)(D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES +
				D_RSRX_CODEC_RASTA_SR_HEADER_BYTES),
		"rasta redundancy encode length contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.usReserve == 0U,
		"rasta redundancy encode reserve contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.uSequenceNumber == 15U,
		"rasta redundancy encode sequence contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.puCarriedPacket == auPayload,
		"rasta redundancy encode carried pointer contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.xCarriedPacketLength ==
			D_RSRX_CODEC_RASTA_SR_HEADER_BYTES,
		"rasta redundancy encode carried length contract");
	vAssertTrue(xRastaRedundancyEncodeRequest.pxCrcProfile == pxExpectedRedundancyCrcProfile,
		"rasta redundancy encode crc profile contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.usPacketLength ==
			xRastaRedundancyEncodeRequest.usPacketLength,
		"rasta redundancy decoded length contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.usReserve == 0U,
		"rasta redundancy decoded reserve contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.uSequenceNumber ==
			xRastaRedundancyEncodeRequest.uSequenceNumber,
		"rasta redundancy decoded sequence contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.xCarriedPacketLength ==
			D_RSRX_CODEC_RASTA_SR_HEADER_BYTES,
		"rasta redundancy decoded carried length contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.auCarriedPacket[0] == auPayload[0],
		"rasta redundancy decoded carried storage contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.xCrcLength == xExpectedNoRedundancyCrcBytes,
		"rasta redundancy decoded crc length contract");
	vAssertTrue(xRastaRedundancyDecodedPacket.uCrcPresent == uExpectedNoRedundancyCrcPresent,
		"rasta redundancy decoded crc present contract");
	vAssertTrue(xRastaSrTimestampPolicy.uCurrentTimestamp == 1000U, "rasta sr timestamp current field contract");
	vAssertTrue(xRastaSrTimestampPolicy.uAcceptedPastWindow == 100U, "rasta sr timestamp past window field contract");
	vAssertTrue(xRastaSrTimestampPolicy.uAcceptedFutureWindow == 10U, "rasta sr timestamp future window field contract");
	vAssertTrue(xRastaSrTimestampPolicy.uLastAcceptedTimestamp == uExpectedLastAcceptedTimestamp, "rasta sr timestamp last accepted field contract");
	vAssertTrue(xRastaSrIdentityPolicy.uExpectedReceiverId == xRastaSrEncodeRequest.uReceiverId, "rasta sr identity receiver field contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xRastaSrIdentityPolicy.uExpectedSenderId == xRastaSrEncodeRequest.uSenderId, "rasta sr identity sender field contract");
	vAssertTrue(xExpectedRastaSrHandoffPointerSize == sizeof(pfRastaSrHandoff), "rasta sr timestamp handoff function pointer contract");
	vAssertTrue(xExpectedRastaSrIdentityHandoffPointerSize == sizeof(pfRastaSrIdentityHandoff), "rasta sr identity handoff function pointer contract");

	(void)printf("rsrx_codec_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
