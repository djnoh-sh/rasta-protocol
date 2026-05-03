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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "unsupported message reject");
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
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_BUFFER_TOO_SMALL, "small buffer reject");
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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_DECODE_ERROR, "reserved header byte reject");
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
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null payload with length reject");
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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_DECODE_ERROR, "trailing byte reject");
}

int main(void)
{
	vTestEncodeDecodeRoundTrip();
	vTestDecodeRejectsUnsupportedMessage();
	vTestEncodeRejectsSmallBuffer();
	vTestDecodeRejectsReservedHeaderBytes();
	vTestEncodeRejectsNullPayloadWithLength();
	vTestDecodeRejectsTrailingBytes();

	(void)printf("rsrx_codec_test: all tests passed\n");

	return EXIT_SUCCESS;
}
