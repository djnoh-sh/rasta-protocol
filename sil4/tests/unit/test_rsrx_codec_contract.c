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

int main(void)
{
	uint8_t auPayload[4] = { 0x11U, 0x22U, 0x33U, 0x44U };
	uint8_t auBuffer[32] = { 0U };
	uint32_t uExpectedEncodedLength;
	rsrx_decode_frame_fn pfExpectedDecode;
	rsrx_decoded_message_t xDecodedMessage;
	rsrx_encode_request_t xEncodeRequest;
	rsrx_encode_buffer_t xEncodeBuffer;
	rsrx_codec_port_t xCodecPort;

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
	xEncodeBuffer.xEncodedLength = uExpectedEncodedLength;

	xCodecPort.pfEncode = (rsrx_encode_message_fn)0;
	pfExpectedDecode = (rsrx_decode_frame_fn)0;
	xCodecPort.pfDecode = pfExpectedDecode;

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

	(void)printf("rsrx_codec_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
