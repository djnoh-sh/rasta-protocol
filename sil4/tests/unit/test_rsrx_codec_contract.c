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
	xEncodeBuffer.xEncodedLength = 0U;

	xCodecPort.pfEncode = (rsrx_encode_message_fn)0;
	xCodecPort.pfDecode = (rsrx_decode_frame_fn)0;

	vAssertTrue(xDecodedMessage.eSuggestedEvent == RSRX_EVENT_VALID_DATA, "decoded message event contract");
	vAssertTrue(xEncodeRequest.eMessageType == RSRX_MESSAGE_TYPE_CONNECT_REQUEST, "encode request type contract");
	vAssertTrue(xEncodeBuffer.xBufferCapacity == sizeof(auBuffer), "encode buffer contract");
	vAssertTrue(xCodecPort.pfEncode == (rsrx_encode_message_fn)0, "codec port layout contract");

	(void)printf("rsrx_codec_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
