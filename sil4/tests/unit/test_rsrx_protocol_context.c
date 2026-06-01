#include <stdio.h>
#include <stdlib.h>

#include "rsrx_protocol_context.h"

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

static uint32_t uReadUint32BigEndian(
	const uint8_t * puBuffer)
{
	return ((uint32_t)puBuffer[0] << 24) |
		((uint32_t)puBuffer[1] << 16) |
		((uint32_t)puBuffer[2] << 8) |
		(uint32_t)puBuffer[3];
}

static void vSeedEncodeRequest(
	rsrx_encode_request_t * pxRequest)
{
	static const uint8_t auStalePayload[1] = { 0xA5U };

	pxRequest->eMessageType = RSRX_MESSAGE_TYPE_DATA;
	pxRequest->eReason = RSRX_REASON_DATA_ACCEPTED;
	pxRequest->uSequenceNumber = 99U;
	pxRequest->uConfirmationNumber = 88U;
	pxRequest->puPayload = auStalePayload;
	pxRequest->xPayloadLength = sizeof(auStalePayload);
}

static void vAssertEncodeRequestCleared(
	const rsrx_encode_request_t * pxRequest,
	const char * pcLabel)
{
	(void)pcLabel;
	vAssertTrue(pxRequest->eMessageType == RSRX_MESSAGE_TYPE_INVALID, "encode request cleared message type");
	vAssertTrue(pxRequest->eReason == RSRX_REASON_NONE, "encode request cleared reason");
	vAssertTrue(pxRequest->uSequenceNumber == 0U, "encode request cleared sequence");
	vAssertTrue(pxRequest->uConfirmationNumber == 0U, "encode request cleared confirmation");
	vAssertTrue(pxRequest->puPayload == (const uint8_t *)0, "encode request cleared payload");
	vAssertTrue(pxRequest->xPayloadLength == 0U, "encode request cleared payload length");
}

static void vTestOutboundSequenceProgression(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;
	static const uint8_t auPayload[2] = { 0x11U, 0x22U };

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_CONNECT_REQUEST,
		RSRX_REASON_CONNECT_REQUESTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "first encode request");
	vAssertTrue(xRequest.uSequenceNumber == 1U, "first sequence");
	vAssertTrue(xRequest.uConfirmationNumber == 0U, "first confirmation");

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_DATA,
		RSRX_REASON_DATA_ACCEPTED,
		auPayload,
		sizeof(auPayload),
		&xRequest) == RSRX_STATUS_OK, "second encode request");
	vAssertTrue(xRequest.uSequenceNumber == 2U, "second sequence");
	vAssertTrue(xRequest.xPayloadLength == sizeof(auPayload), "data payload length");
}

static void vTestProtocolContextInitClearsBaseline(void)
{
	rsrx_protocol_context_t xContext;

	xContext.uNextTxSequenceNumber = 42U;
	xContext.uLastRxSequenceNumber = 41U;
	xContext.uLastTxConfirmationNumber = 40U;
	xContext.uLastRemoteConfirmationNumber = 39U;
	xContext.uLastRetransmissionRequestTxSequenceNumber = 38U;
	xContext.uRetransmissionBaseSequenceNumber = 37U;
	xContext.uRetransmissionPending = 1U;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init baseline");
	vAssertTrue(xContext.uNextTxSequenceNumber == 1U, "init baseline next tx");
	vAssertTrue(xContext.uLastRxSequenceNumber == 0U, "init baseline last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 0U, "init baseline tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 0U, "init baseline remote confirmation");
	vAssertTrue(
		xContext.uLastRetransmissionRequestTxSequenceNumber == 0U,
		"init baseline retransmission request tx");
	vAssertTrue(
		xContext.uRetransmissionBaseSequenceNumber == 0U,
		"init baseline retransmission base");
	vAssertTrue(xContext.uRetransmissionPending == 0U, "init baseline retransmission pending");
}

static void vTestInboundConfirmationTracking(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
	xMessage.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	xMessage.eSuggestedEvent = RSRX_EVENT_HANDSHAKE_SUCCESS;
	xMessage.eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	xMessage.uSequenceNumber = 9U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record inbound");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_HEARTBEAT,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "heartbeat request");
	vAssertTrue(xRequest.uConfirmationNumber == 9U, "confirmation tracks inbound sequence");
}

static void vTestOutboundSequenceWrapRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "wrap guard protocol init");
	xContext.uNextTxSequenceNumber = UINT32_MAX - 1U;

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_HEARTBEAT,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "wrap guard last safe encode request");
	vAssertTrue(xRequest.uSequenceNumber == (UINT32_MAX - 1U), "wrap guard last safe sequence");
	vAssertTrue(xContext.uNextTxSequenceNumber == UINT32_MAX, "wrap guard next sequence reaches limit");

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_HEARTBEAT,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_REJECTED, "wrap guard rejects overflow boundary");
	vAssertTrue(xContext.uNextTxSequenceNumber == UINT32_MAX, "wrap guard sequence retained on reject");
	vAssertEncodeRequestCleared(&xRequest, "wrap guard reject clears request");
}

static void vTestInvalidOutboundMessageTypeRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "invalid outbound type init");
	vSeedEncodeRequest(&xRequest);
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_INVALID,
		RSRX_REASON_NONE,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_INVALID_ARGUMENT, "invalid outbound type rejected");
	vAssertTrue(xContext.uNextTxSequenceNumber == 1U, "invalid outbound type keeps next tx sequence");
	vAssertEncodeRequestCleared(&xRequest, "invalid outbound type clears request");
}

static void vTestRetransmissionRequestPayload(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 12U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record inbound");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "retransmission request");
	vAssertTrue(xRequest.xPayloadLength == 4U, "retransmission payload length");
	vAssertTrue(uReadUint32BigEndian(xRequest.puPayload) == 13U, "retransmission base sequence");

	vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "retransmission request after clear");
	vAssertTrue(uReadUint32BigEndian(xRequest.puPayload) == 13U, "retransmission base recomputed");
}

static void vTestClearRetransmissionPreservesSequenceState(void)
{
	rsrx_protocol_context_t xContext;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "clear state protocol init");
	xContext.uNextTxSequenceNumber = 6U;
	xContext.uLastRxSequenceNumber = 4U;
	xContext.uLastTxConfirmationNumber = 4U;
	xContext.uLastRemoteConfirmationNumber = 5U;
	xContext.uLastRetransmissionRequestTxSequenceNumber = 3U;
	xContext.uRetransmissionBaseSequenceNumber = 5U;
	xContext.uRetransmissionPending = 1U;

	vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission state");
	vAssertTrue(xContext.uNextTxSequenceNumber == 6U, "clear keeps next tx sequence");
	vAssertTrue(xContext.uLastRxSequenceNumber == 4U, "clear keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 4U, "clear keeps tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 5U, "clear keeps remote confirmation");
	vAssertTrue(xContext.uLastRetransmissionRequestTxSequenceNumber == 0U, "clear resets retransmission request tx");
	vAssertTrue(xContext.uRetransmissionBaseSequenceNumber == 0U, "clear resets retransmission base");
	vAssertTrue(xContext.uRetransmissionPending == 0U, "clear resets retransmission pending");

	vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission idempotent");
	vAssertTrue(xContext.uNextTxSequenceNumber == 6U, "idempotent clear keeps next tx sequence");
	vAssertTrue(xContext.uLastRxSequenceNumber == 4U, "idempotent clear keeps last rx");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 5U, "idempotent clear keeps remote confirmation");
	vAssertTrue(xContext.uRetransmissionPending == 0U, "idempotent clear keeps pending reset");
}

static void vTestInboundConfirmationValidation(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_CONNECT_REQUEST,
		RSRX_REASON_CONNECT_REQUESTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "first outbound");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_HEARTBEAT,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "second outbound");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_HEARTBEAT;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_HEARTBEAT;
	xMessage.eReason = RSRX_REASON_HEARTBEAT_ACCEPTED;
	xMessage.uSequenceNumber = 1U;
	xMessage.uConfirmationNumber = 3U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve invalid confirmation");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "invalid confirmation becomes protocol error");

	xMessage.uConfirmationNumber = 2U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve valid confirmation");
	vAssertTrue(eEvent == RSRX_EVENT_VALID_HEARTBEAT, "valid confirmation accepted");
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record valid confirmation");

	xMessage.uSequenceNumber = 2U;
	xMessage.uConfirmationNumber = 1U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve regressing confirmation");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "regressing confirmation rejected");
}

static void vTestInvalidConfirmationRecordRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;
	rsrx_decoded_message_t xMessage;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "record guard protocol init");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_CONNECT_REQUEST,
		RSRX_REASON_CONNECT_REQUESTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "record guard first outbound");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_HEARTBEAT,
		RSRX_REASON_HEARTBEAT_ACCEPTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "record guard second outbound");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 1U;
	xMessage.uConfirmationNumber = 2U;
	xMessage.xPayloadLength = 0U;
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record guard valid baseline");
	vAssertTrue(xContext.uLastRxSequenceNumber == 1U, "record guard baseline last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 1U, "record guard baseline tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 2U, "record guard baseline remote confirmation");

	xMessage.uSequenceNumber = 2U;
	xMessage.uConfirmationNumber = 3U;
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_REJECTED, "record guard high confirmation rejected");
	vAssertTrue(xContext.uLastRxSequenceNumber == 1U, "record guard high confirmation keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 1U, "record guard high confirmation keeps tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 2U, "record guard high confirmation keeps remote confirmation");

	xMessage.uConfirmationNumber = 1U;
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_REJECTED, "record guard regressing confirmation rejected");
	vAssertTrue(xContext.uLastRxSequenceNumber == 1U, "record guard regressing confirmation keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 1U, "record guard regressing confirmation keeps tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 2U, "record guard regressing confirmation keeps remote confirmation");
}

static void vTestInvalidInboundMessageTypeRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "invalid inbound type init");
	xContext.uNextTxSequenceNumber = 4U;
	xContext.uLastRxSequenceNumber = 7U;
	xContext.uLastTxConfirmationNumber = 7U;
	xContext.uLastRemoteConfirmationNumber = 3U;
	xContext.uRetransmissionPending = 1U;
	xContext.uRetransmissionBaseSequenceNumber = 8U;
	xContext.uLastRetransmissionRequestTxSequenceNumber = 2U;

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	xMessage.eSuggestedEvent = RSRX_EVENT_INVALID;
	xMessage.eReason = RSRX_REASON_NONE;
	xMessage.uSequenceNumber = 99U;
	xMessage.uConfirmationNumber = 99U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(
		rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_INVALID_ARGUMENT,
		"invalid inbound type rejected");
	vAssertTrue(xContext.uNextTxSequenceNumber == 4U, "invalid inbound type keeps next tx sequence");
	vAssertTrue(xContext.uLastRxSequenceNumber == 7U, "invalid inbound type keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 7U, "invalid inbound type keeps tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 3U, "invalid inbound type keeps remote confirmation");
	vAssertTrue(xContext.uRetransmissionPending == 1U, "invalid inbound type keeps retransmission pending");
	vAssertTrue(xContext.uRetransmissionBaseSequenceNumber == 8U, "invalid inbound type keeps retransmission base");
	vAssertTrue(
		xContext.uLastRetransmissionRequestTxSequenceNumber == 2U,
		"invalid inbound type keeps retransmission request tx");
}

static void vTestRecoverySuccessResolution(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 3U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "start retransmission pending");

	xMessage.uSequenceNumber = 4U;
	xMessage.uConfirmationNumber = 1U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve recovery success");
	vAssertTrue(eEvent == RSRX_EVENT_RECOVERY_SUCCESS, "base sequence resolves to recovery success");

	xMessage.uConfirmationNumber = 0U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve unconfirmed recovery");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "unconfirmed retransmission recovery rejected");

	xMessage.uSequenceNumber = 5U;
	xMessage.uConfirmationNumber = 1U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve recovery gap");
	vAssertTrue(eEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "higher sequence during retransmission remains gap");

	xMessage.uSequenceNumber = 3U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve stale during retransmission");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "lower sequence during retransmission is protocol error");
}

static void vTestRetransmissionOrderingMatrix(void)
{
	typedef struct
	{
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;
	rsrx_encode_request_t xRequest;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{ 4U, 1U, RSRX_EVENT_RECOVERY_SUCCESS },
		{ 4U, 0U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 4U, 2U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 5U, 1U, RSRX_EVENT_SEQUENCE_GAP_DETECTED },
		{ 3U, 1U, RSRX_EVENT_PROTOCOL_ERROR }
	};

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 3U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");
	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "start retransmission pending");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.xPayloadLength = 0U;

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;
		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"retransmission ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "retransmission ordering matrix event");
	}
}

static void vTestSteadyStateOrderingMatrix(void)
{
	typedef struct
	{
		uint32_t uNextTxSequenceNumber;
		uint32_t uLastRxSequenceNumber;
		uint32_t uLastRemoteConfirmationNumber;
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{ 3U, 1U, 0U, 2U, 0U, RSRX_EVENT_VALID_DATA },
		{ 3U, 1U, 0U, 3U, 0U, RSRX_EVENT_SEQUENCE_GAP_DETECTED },
		{ 3U, 1U, 0U, 1U, 0U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 3U, 1U, 0U, 2U, 3U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 4U, 2U, 2U, 3U, 1U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 4U, 2U, 2U, 3U, 2U, RSRX_EVENT_VALID_DATA }
	};

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.xPayloadLength = 0U;

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
		xContext.uNextTxSequenceNumber = axCases[uIndex].uNextTxSequenceNumber;
		xContext.uLastRxSequenceNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastTxConfirmationNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastRemoteConfirmationNumber = axCases[uIndex].uLastRemoteConfirmationNumber;

		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;

		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"steady-state ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "steady-state ordering matrix event");
	}
}

static void vTestPostRecoveryOrderingMatrix(void)
{
	typedef struct
	{
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{ 5U, 1U, RSRX_EVENT_VALID_DATA },
		{ 5U, 3U, RSRX_EVENT_VALID_DATA },
		{ 5U, 0U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 5U, 4U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 6U, 1U, RSRX_EVENT_SEQUENCE_GAP_DETECTED }
	};

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

		xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
		xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
		xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
		xMessage.uSequenceNumber = 3U;
		xMessage.uConfirmationNumber = 0U;
		xMessage.xPayloadLength = 0U;
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "start retransmission pending");

		xMessage.uSequenceNumber = 4U;
		xMessage.uConfirmationNumber = 1U;
		vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve recovery success");
		vAssertTrue(eEvent == RSRX_EVENT_RECOVERY_SUCCESS, "recovery success accepted");
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record recovery success");
		vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "first outbound after recovery");
		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "second outbound after recovery");

		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;
		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"post-recovery ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "post-recovery ordering matrix event");
	}
}

static void vTestRepeatedGapRetransmissionProgression(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 3U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "first retransmission request");
	vAssertTrue(xRequest.uSequenceNumber == 1U, "first retransmission tx sequence");
	vAssertTrue(uReadUint32BigEndian(xRequest.puPayload) == 4U, "first retransmission base sequence");

	xMessage.uSequenceNumber = 5U;
	xMessage.uConfirmationNumber = 1U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve repeated gap");
	vAssertTrue(eEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "repeated gap remains gap");

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_OK, "second retransmission request");
	vAssertTrue(xRequest.uSequenceNumber == 2U, "second retransmission tx sequence");
	vAssertTrue(uReadUint32BigEndian(xRequest.puPayload) == 4U, "repeated gap keeps retransmission base");

	xMessage.uSequenceNumber = 4U;
	xMessage.uConfirmationNumber = 2U;
	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve confirmed recovery after repeated gap");
	vAssertTrue(eEvent == RSRX_EVENT_RECOVERY_SUCCESS, "confirmed recovery after repeated gap succeeds");
}

static void vTestRepeatedGapRecoveryOrderingMatrix(void)
{
	typedef struct
	{
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{ 4U, 1U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 4U, 2U, RSRX_EVENT_RECOVERY_SUCCESS },
		{ 4U, 3U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 5U, 2U, RSRX_EVENT_SEQUENCE_GAP_DETECTED },
		{ 3U, 2U, RSRX_EVENT_PROTOCOL_ERROR }
	};

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

		xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
		xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
		xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
		xMessage.uSequenceNumber = 3U;
		xMessage.uConfirmationNumber = 0U;
		xMessage.xPayloadLength = 0U;
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "first retransmission request");

		xMessage.uSequenceNumber = 5U;
		xMessage.uConfirmationNumber = 1U;
		vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve repeated gap");
		vAssertTrue(eEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "repeated gap remains gap");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "second retransmission request");

		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;
		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"repeated-gap recovery ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "repeated-gap recovery ordering matrix event");
	}
}

static void vTestRepeatedGapPostRecoveryOrderingMatrix(void)
{
	typedef struct
	{
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{ 5U, 2U, RSRX_EVENT_VALID_DATA },
		{ 5U, 4U, RSRX_EVENT_VALID_DATA },
		{ 5U, 1U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 5U, 5U, RSRX_EVENT_PROTOCOL_ERROR },
		{ 6U, 2U, RSRX_EVENT_SEQUENCE_GAP_DETECTED }
	};

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

		xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
		xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
		xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
		xMessage.uSequenceNumber = 3U;
		xMessage.uConfirmationNumber = 0U;
		xMessage.xPayloadLength = 0U;
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "first retransmission request");

		xMessage.uSequenceNumber = 5U;
		xMessage.uConfirmationNumber = 1U;
		vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve repeated gap");
		vAssertTrue(eEvent == RSRX_EVENT_SEQUENCE_GAP_DETECTED, "repeated gap remains gap");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "second retransmission request");

		xMessage.uSequenceNumber = 4U;
		xMessage.uConfirmationNumber = 2U;
		vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve latest recovery success");
		vAssertTrue(eEvent == RSRX_EVENT_RECOVERY_SUCCESS, "latest recovery success accepted");
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record recovery success");
		vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "first outbound after repeated-gap recovery");
		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "second outbound after repeated-gap recovery");

		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;
		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"repeated-gap post-recovery ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "repeated-gap post-recovery ordering matrix event");
	}
}

static void vTestSequencedMessageFamilyOrderingMatrix(void)
{
	typedef struct
	{
		rsrx_message_type_t eMessageType;
		rsrx_event_t eSuggestedEvent;
		uint32_t uNextTxSequenceNumber;
		uint32_t uLastRxSequenceNumber;
		uint32_t uLastRemoteConfirmationNumber;
		uint32_t uRetransmissionPending;
		uint32_t uRetransmissionBaseSequenceNumber;
		uint32_t uLastRetransmissionRequestTxSequenceNumber;
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{
			RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
			RSRX_EVENT_HANDSHAKE_SUCCESS,
			1U, 0U, 0U, 0U, 0U, 0U,
			1U, 0U, RSRX_EVENT_HANDSHAKE_SUCCESS
		},
		{
			RSRX_MESSAGE_TYPE_HEARTBEAT,
			RSRX_EVENT_VALID_HEARTBEAT,
			1U, 0U, 0U, 0U, 0U, 0U,
			2U, 0U, RSRX_EVENT_SEQUENCE_GAP_DETECTED
		},
		{
			RSRX_MESSAGE_TYPE_HEARTBEAT,
			RSRX_EVENT_VALID_HEARTBEAT,
			3U, 1U, 0U, 0U, 0U, 0U,
			2U, 0U, RSRX_EVENT_VALID_HEARTBEAT
		},
		{
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			3U, 1U, 0U, 0U, 0U, 0U,
			1U, 0U, RSRX_EVENT_PROTOCOL_ERROR
		},
		{
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_EVENT_SEQUENCE_GAP_DETECTED,
			3U, 1U, 0U, 0U, 0U, 0U,
			2U, 0U, RSRX_EVENT_SEQUENCE_GAP_DETECTED
		},
		{
			RSRX_MESSAGE_TYPE_HEARTBEAT,
			RSRX_EVENT_VALID_HEARTBEAT,
			2U, 3U, 0U, 1U, 4U, 1U,
			4U, 1U, RSRX_EVENT_RECOVERY_SUCCESS
		},
		{
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_EVENT_SEQUENCE_GAP_DETECTED,
			2U, 3U, 0U, 1U, 4U, 1U,
			4U, 0U, RSRX_EVENT_PROTOCOL_ERROR
		},
		{
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			2U, 3U, 0U, 1U, 4U, 1U,
			5U, 1U, RSRX_EVENT_SEQUENCE_GAP_DETECTED
		}
	};

	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.xPayloadLength = 0U;

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
		xContext.uNextTxSequenceNumber = axCases[uIndex].uNextTxSequenceNumber;
		xContext.uLastRxSequenceNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastTxConfirmationNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastRemoteConfirmationNumber = axCases[uIndex].uLastRemoteConfirmationNumber;
		xContext.uRetransmissionPending = axCases[uIndex].uRetransmissionPending;
		xContext.uRetransmissionBaseSequenceNumber = axCases[uIndex].uRetransmissionBaseSequenceNumber;
		xContext.uLastRetransmissionRequestTxSequenceNumber =
			axCases[uIndex].uLastRetransmissionRequestTxSequenceNumber;

		xMessage.eMessageType = axCases[uIndex].eMessageType;
		xMessage.eSuggestedEvent = axCases[uIndex].eSuggestedEvent;
		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;

		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"sequenced message family ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "sequenced message family ordering matrix event");
	}
}

static void vTestPostRecoveryMessageFamilyOrderingMatrix(void)
{
	typedef struct
	{
		rsrx_message_type_t eMessageType;
		rsrx_event_t eSuggestedEvent;
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{
			RSRX_MESSAGE_TYPE_HEARTBEAT,
			RSRX_EVENT_VALID_HEARTBEAT,
			5U, 1U, RSRX_EVENT_VALID_HEARTBEAT
		},
		{
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			5U, 3U, RSRX_EVENT_VALID_DATA
		},
		{
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_EVENT_SEQUENCE_GAP_DETECTED,
			5U, 3U, RSRX_EVENT_SEQUENCE_GAP_DETECTED
		},
		{
			RSRX_MESSAGE_TYPE_HEARTBEAT,
			RSRX_EVENT_VALID_HEARTBEAT,
			5U, 0U, RSRX_EVENT_PROTOCOL_ERROR
		},
		{
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_EVENT_VALID_DATA,
			5U, 4U, RSRX_EVENT_PROTOCOL_ERROR
		},
		{
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_EVENT_SEQUENCE_GAP_DETECTED,
			6U, 1U, RSRX_EVENT_SEQUENCE_GAP_DETECTED
		}
	};

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

		xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
		xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
		xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
		xMessage.uSequenceNumber = 3U;
		xMessage.uConfirmationNumber = 0U;
		xMessage.xPayloadLength = 0U;
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record baseline");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
			RSRX_REASON_SEQUENCE_GAP_DETECTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "start retransmission pending");

		xMessage.uSequenceNumber = 4U;
		xMessage.uConfirmationNumber = 1U;
		vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve recovery success");
		vAssertTrue(eEvent == RSRX_EVENT_RECOVERY_SUCCESS, "recovery success accepted");
		vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record recovery success");
		vAssertTrue(rsrx_protocol_context_clear_retransmission(&xContext) == RSRX_STATUS_OK, "clear retransmission");

		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "first outbound after recovery");
		vAssertTrue(rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_OK, "second outbound after recovery");

		xMessage.eMessageType = axCases[uIndex].eMessageType;
		xMessage.eSuggestedEvent = axCases[uIndex].eSuggestedEvent;
		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;
		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"post-recovery message family ordering matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "post-recovery message family ordering matrix event");
	}
}

static void vTestUnsequencedMessageFamilyPassThroughMatrix(void)
{
	typedef struct
	{
		rsrx_message_type_t eMessageType;
		rsrx_event_t eSuggestedEvent;
		rsrx_reason_code_t eReason;
		uint32_t uNextTxSequenceNumber;
		uint32_t uLastRxSequenceNumber;
		uint32_t uLastRemoteConfirmationNumber;
		uint32_t uRetransmissionPending;
		uint32_t uRetransmissionBaseSequenceNumber;
		uint32_t uLastRetransmissionRequestTxSequenceNumber;
		uint32_t uSequenceNumber;
		uint32_t uConfirmationNumber;
		rsrx_event_t eExpectedEvent;
	} test_case_t;

	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;
	uint32_t uIndex;
	static const test_case_t axCases[] =
	{
		{
			RSRX_MESSAGE_TYPE_CONNECT_REQUEST,
			RSRX_EVENT_CONNECT_REQUEST,
			RSRX_REASON_CONNECT_REQUESTED,
			1U, 0U, 0U, 0U, 0U, 0U,
			7U, 9U, RSRX_EVENT_CONNECT_REQUEST
		},
		{
			RSRX_MESSAGE_TYPE_DISCONNECT,
			RSRX_EVENT_DISCONNECT_REQUEST,
			RSRX_REASON_DISCONNECT_REQUESTED,
			4U, 3U, 2U, 0U, 0U, 0U,
			99U, 77U, RSRX_EVENT_DISCONNECT_REQUEST
		},
		{
			RSRX_MESSAGE_TYPE_DIAGNOSTIC,
			RSRX_EVENT_INVALID_MESSAGE,
			RSRX_REASON_INVALID_MESSAGE_RECEIVED,
			2U, 8U, 1U, 1U, 9U, 1U,
			5U, 0U, RSRX_EVENT_INVALID_MESSAGE
		}
	};

	xMessage.xPayloadLength = 0U;

	for(uIndex = 0U; uIndex < (sizeof(axCases) / sizeof(axCases[0])); ++uIndex)
	{
		vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");
		xContext.uNextTxSequenceNumber = axCases[uIndex].uNextTxSequenceNumber;
		xContext.uLastRxSequenceNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastTxConfirmationNumber = axCases[uIndex].uLastRxSequenceNumber;
		xContext.uLastRemoteConfirmationNumber = axCases[uIndex].uLastRemoteConfirmationNumber;
		xContext.uRetransmissionPending = axCases[uIndex].uRetransmissionPending;
		xContext.uRetransmissionBaseSequenceNumber = axCases[uIndex].uRetransmissionBaseSequenceNumber;
		xContext.uLastRetransmissionRequestTxSequenceNumber =
			axCases[uIndex].uLastRetransmissionRequestTxSequenceNumber;

		xMessage.eMessageType = axCases[uIndex].eMessageType;
		xMessage.eSuggestedEvent = axCases[uIndex].eSuggestedEvent;
		xMessage.eReason = axCases[uIndex].eReason;
		xMessage.uSequenceNumber = axCases[uIndex].uSequenceNumber;
		xMessage.uConfirmationNumber = axCases[uIndex].uConfirmationNumber;

		vAssertTrue(
			rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
			"unsequenced message family pass-through matrix resolve");
		vAssertTrue(eEvent == axCases[uIndex].eExpectedEvent, "unsequenced message family pass-through matrix event");
		vAssertTrue(
			rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK,
			"unsequenced message family pass-through matrix record");
		vAssertTrue(
			xContext.uLastRxSequenceNumber == axCases[uIndex].uLastRxSequenceNumber,
			"unsequenced message family record keeps last rx");
		vAssertTrue(
			xContext.uLastTxConfirmationNumber == axCases[uIndex].uLastRxSequenceNumber,
			"unsequenced message family record keeps tx confirmation");
		vAssertTrue(
			xContext.uLastRemoteConfirmationNumber == axCases[uIndex].uLastRemoteConfirmationNumber,
			"unsequenced message family record keeps remote confirmation");
		vAssertTrue(
			xContext.uRetransmissionPending == axCases[uIndex].uRetransmissionPending,
			"unsequenced message family record keeps retransmission pending");
	}
}

static void vTestConnectRequestRemainsUnsequencedBaseline(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "connect request baseline init");
	xContext.uNextTxSequenceNumber = 5U;
	xContext.uLastRxSequenceNumber = 0U;
	xContext.uLastTxConfirmationNumber = 0U;
	xContext.uLastRemoteConfirmationNumber = 0U;

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_REQUEST;
	xMessage.eSuggestedEvent = RSRX_EVENT_CONNECT_REQUEST;
	xMessage.eReason = RSRX_REASON_CONNECT_REQUESTED;
	xMessage.uSequenceNumber = 7U;
	xMessage.uConfirmationNumber = 9U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
		"connect request baseline resolve");
	vAssertTrue(eEvent == RSRX_EVENT_CONNECT_REQUEST, "connect request baseline event");
	vAssertTrue(
		rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK,
		"connect request baseline record");
	vAssertTrue(xContext.uLastRxSequenceNumber == 0U, "connect request baseline keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 0U, "connect request baseline keeps tx confirmation");
	vAssertTrue(xContext.uLastRemoteConfirmationNumber == 0U, "connect request baseline keeps remote confirmation");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	xMessage.eSuggestedEvent = RSRX_EVENT_HANDSHAKE_SUCCESS;
	xMessage.eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	xMessage.uSequenceNumber = 1U;
	xMessage.uConfirmationNumber = 0U;

	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK,
		"connect response after unsequenced connect request resolve");
	vAssertTrue(
		eEvent == RSRX_EVENT_HANDSHAKE_SUCCESS,
		"first sequenced inbound remains connect response after unsequenced connect request");
}

static void vTestDuplicateInboundSequenceRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	xMessage.eSuggestedEvent = RSRX_EVENT_HANDSHAKE_SUCCESS;
	xMessage.eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	xMessage.uSequenceNumber = 1U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record handshake");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 2U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 1U;

	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve first data");
	vAssertTrue(eEvent == RSRX_EVENT_VALID_DATA, "first data accepted");
	vAssertTrue(rsrx_protocol_context_record_inbound_message(&xContext, &xMessage) == RSRX_STATUS_OK, "record first data");

	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve duplicate data");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "duplicate data rejected");
}

static void vTestInitialZeroSequenceRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "protocol init");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_CONNECT_RESPONSE;
	xMessage.eSuggestedEvent = RSRX_EVENT_HANDSHAKE_SUCCESS;
	xMessage.eReason = RSRX_REASON_HANDSHAKE_COMPLETED;
	xMessage.uSequenceNumber = 0U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;

	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve zero initial sequence");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "zero initial sequence rejected");
}

static void vTestInboundSequenceWrapRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "inbound wrap protocol init");
	xContext.uLastRxSequenceNumber = UINT32_MAX;

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 0U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 1U;

	vAssertTrue(rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) == RSRX_STATUS_OK, "resolve inbound wrap sequence");
	vAssertTrue(eEvent == RSRX_EVENT_PROTOCOL_ERROR, "inbound wrap sequence rejected");
}

static void vTestRetransmissionBaseWrapRejected(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "retransmission base wrap protocol init");
	xContext.uLastRxSequenceNumber = UINT32_MAX;
	xContext.uLastTxConfirmationNumber = UINT32_MAX;
	vSeedEncodeRequest(&xRequest);

	vAssertTrue(rsrx_protocol_context_build_encode_request(
		&xContext,
		RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
		RSRX_REASON_SEQUENCE_GAP_DETECTED,
		(const uint8_t *)0,
		0U,
		&xRequest) == RSRX_STATUS_REJECTED, "retransmission base wrap rejected");
	vAssertTrue(xContext.uRetransmissionPending == 0U, "retransmission base wrap keeps pending clear");
	vAssertTrue(xContext.uRetransmissionBaseSequenceNumber == 0U, "retransmission base wrap keeps base clear");
	vAssertTrue(xContext.uLastRetransmissionRequestTxSequenceNumber == 0U, "retransmission base wrap keeps request tx clear");
	vAssertTrue(xContext.uNextTxSequenceNumber == 1U, "retransmission base wrap keeps next tx sequence");
	vAssertEncodeRequestCleared(&xRequest, "retransmission base wrap clears request");
}

static void vTestEncodeFailureClearsRequest(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_encode_request_t xRequest;

	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "encode clear protocol init");

	vSeedEncodeRequest(&xRequest);
	vAssertTrue(
		rsrx_protocol_context_build_encode_request(
			(rsrx_protocol_context_t *)0,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_INVALID_ARGUMENT,
		"encode clear null context rejected");
	vAssertEncodeRequestCleared(&xRequest, "null context clears request");

	vSeedEncodeRequest(&xRequest);
	xContext.uNextTxSequenceNumber = UINT32_MAX;
	vAssertTrue(
		rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_REJECTED,
		"encode clear wrap rejected");
	vAssertTrue(xContext.uNextTxSequenceNumber == UINT32_MAX, "encode clear wrap keeps next tx");
	vAssertEncodeRequestCleared(&xRequest, "wrap reject clears request");
}

static void vTestInvalidArguments(void)
{
	rsrx_protocol_context_t xContext;
	rsrx_decoded_message_t xMessage;
	rsrx_encode_request_t xRequest;
	rsrx_event_t eEvent;

	vAssertTrue(rsrx_protocol_context_init((rsrx_protocol_context_t *)0) == RSRX_STATUS_INVALID_ARGUMENT, "null init");
	vAssertTrue(rsrx_protocol_context_init(&xContext) == RSRX_STATUS_OK, "valid init");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_DATA;
	xMessage.eSuggestedEvent = RSRX_EVENT_VALID_DATA;
	xMessage.eReason = RSRX_REASON_DATA_ACCEPTED;
	xMessage.uSequenceNumber = 1U;
	xMessage.uConfirmationNumber = 0U;
	xMessage.xPayloadLength = 0U;
	eEvent = RSRX_EVENT_INIT_FAILURE;

	vAssertTrue(
		rsrx_protocol_context_record_inbound_message((rsrx_protocol_context_t *)0, &xMessage) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"null record context");
	vAssertTrue(
		rsrx_protocol_context_record_inbound_message(&xContext, (const rsrx_decoded_message_t *)0) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"null inbound");
	vAssertTrue(xContext.uLastRxSequenceNumber == 0U, "null inbound keeps last rx");
	vAssertTrue(xContext.uLastTxConfirmationNumber == 0U, "null inbound keeps tx confirmation");

	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(
			(const rsrx_protocol_context_t *)0,
			&xMessage,
			&eEvent) == RSRX_STATUS_INVALID_ARGUMENT,
		"null resolve context");
	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(&xContext, (const rsrx_decoded_message_t *)0, &eEvent) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"null resolve message");
	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, (rsrx_event_t *)0) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"null resolve event");
	vAssertTrue(eEvent == RSRX_EVENT_INIT_FAILURE, "null resolve keeps event");

	xMessage.eMessageType = RSRX_MESSAGE_TYPE_INVALID;
	vAssertTrue(
		rsrx_protocol_context_resolve_inbound_event(&xContext, &xMessage, &eEvent) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"invalid resolve message type");
	vAssertTrue(eEvent == RSRX_EVENT_INIT_FAILURE, "invalid resolve type keeps event");

	vAssertTrue(
		rsrx_protocol_context_build_encode_request(
			(rsrx_protocol_context_t *)0,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			&xRequest) == RSRX_STATUS_INVALID_ARGUMENT,
		"null encode context");
	vAssertTrue(
		rsrx_protocol_context_build_encode_request(
			&xContext,
			RSRX_MESSAGE_TYPE_DATA,
			RSRX_REASON_DATA_ACCEPTED,
			(const uint8_t *)0,
			0U,
			(rsrx_encode_request_t *)0) == RSRX_STATUS_INVALID_ARGUMENT,
		"null request");
	vAssertTrue(xContext.uNextTxSequenceNumber == 1U, "null encode keeps next tx sequence");

	vAssertTrue(
		rsrx_protocol_context_clear_retransmission((rsrx_protocol_context_t *)0) ==
			RSRX_STATUS_INVALID_ARGUMENT,
		"null clear retransmission context");
}

static void vTestProtocolOrderingCloseoutMatrix(void)
{
	vTestRetransmissionOrderingMatrix();
	vTestSteadyStateOrderingMatrix();
	vTestPostRecoveryOrderingMatrix();
	vTestRepeatedGapRetransmissionProgression();
	vTestRepeatedGapRecoveryOrderingMatrix();
	vTestRepeatedGapPostRecoveryOrderingMatrix();
	vTestSequencedMessageFamilyOrderingMatrix();
	vTestPostRecoveryMessageFamilyOrderingMatrix();
	vTestUnsequencedMessageFamilyPassThroughMatrix();
	vTestConnectRequestRemainsUnsequencedBaseline();
}

int main(void)
{
	vTestOutboundSequenceProgression();
	vTestProtocolContextInitClearsBaseline();
	vTestInboundConfirmationTracking();
	vTestOutboundSequenceWrapRejected();
	vTestInvalidOutboundMessageTypeRejected();
	vTestRetransmissionRequestPayload();
	vTestClearRetransmissionPreservesSequenceState();
	vTestInboundConfirmationValidation();
	vTestInvalidConfirmationRecordRejected();
	vTestInvalidInboundMessageTypeRejected();
	vTestRecoverySuccessResolution();
	vTestProtocolOrderingCloseoutMatrix();
	vTestDuplicateInboundSequenceRejected();
	vTestInitialZeroSequenceRejected();
	vTestInboundSequenceWrapRejected();
	vTestRetransmissionBaseWrapRejected();
	vTestEncodeFailureClearsRequest();
	vTestInvalidArguments();

	(void)printf("rsrx_protocol_context_test: all tests passed\n");
	return EXIT_SUCCESS;
}
