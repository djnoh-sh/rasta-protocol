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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "unsupported message reject");
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
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_BUFFER_TOO_SMALL, "small buffer reject");
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
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "oversized encode payload reject");
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
	xBuffer.xEncodedLength = 0U;

	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE, "unsupported encode message reject");
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
	xBuffer.xEncodedLength = 0U;

	auEncoded[0] = (uint8_t)RSRX_MESSAGE_TYPE_DATA;
	auEncoded[1] = (uint8_t)RSRX_REASON_DATA_ACCEPTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auEncoded;
	xFrame.xPayloadLength = sizeof(auEncoded);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	vAssertTrue(rsrx_codec_encode_message((const rsrx_encode_request_t *)0, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode request reject");
	vAssertTrue(rsrx_codec_encode_message(&xRequest, (rsrx_encode_buffer_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode buffer reject");
	xBuffer.puBuffer = (uint8_t *)0;
	vAssertTrue(rsrx_codec_encode_message(&xRequest, &xBuffer) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null encode output buffer reject");
	vAssertTrue(rsrx_codec_decode_frame((const rsrx_transport_frame_t *)0, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null decode frame reject");
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, (rsrx_decoded_message_t *)0) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null decoded message reject");
	xFrame.puPayload = (const uint8_t *)0;
	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_INVALID_ARGUMENT, "null frame payload reject");
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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_DECODE_ERROR, "short header reject");
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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_DECODE_ERROR, "truncated payload reject");
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

	vAssertTrue(rsrx_codec_decode_frame(&xFrame, &xMessage) == RSRX_CODEC_STATUS_DECODE_ERROR, "oversized declared payload reject");
}

int main(void)
{
	vTestEncodeDecodeRoundTrip();
	vTestDefaultPortEncodeDecodeRoundTrip();
	vTestMaxPayloadEncodeDecodeRoundTrip();
	vTestDecodeRejectsUnsupportedMessage();
	vTestDecodeMapsSupportedMessageTypes();
	vTestEncodeRejectsSmallBuffer();
	vTestEncodeRejectsOversizedPayloadLength();
	vTestEncodeRejectsUnsupportedMessageType();
	vTestDecodeRejectsReservedHeaderBytes();
	vTestEncodeRejectsNullPayloadWithLength();
	vTestRejectsNullArguments();
	vTestDecodeRejectsTrailingBytes();
	vTestDecodeRejectsShortHeader();
	vTestDecodeRejectsTruncatedPayload();
	vTestDecodeRejectsOversizedDeclaredPayload();

	(void)printf("rsrx_codec_test: all tests passed\n");

	return EXIT_SUCCESS;
}
