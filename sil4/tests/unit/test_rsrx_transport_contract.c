#include <stdio.h>
#include <stdlib.h>

#include "rsrx_transport.h"

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
	uint8_t auPayload[3] = { 0x01U, 0x02U, 0x03U };
	uint32_t uExpectedAvailable;
	rsrx_transport_channel_query_fn pfExpectedQuery;
	rsrx_transport_send_request_t xSendRequest;
	rsrx_transport_frame_t xFrame;
	rsrx_transport_channel_state_t xChannelState;
	rsrx_transport_port_t xPort;

	xSendRequest.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xSendRequest.puPayload = auPayload;
	xSendRequest.xPayloadLength = sizeof(auPayload);
	xSendRequest.eReason = RSRX_REASON_CONNECT_REQUESTED;

	xFrame.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	xFrame.puPayload = auPayload;
	xFrame.xPayloadLength = sizeof(auPayload);
	xFrame.eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;

	xChannelState.eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	uExpectedAvailable = 1U;
	xChannelState.uIsAvailable = uExpectedAvailable;

	xPort.pvContext = (void *)0;
	xPort.pfSend = (rsrx_transport_send_fn)0;
	xPort.pfReceive = (rsrx_transport_receive_fn)0;
	pfExpectedQuery = (rsrx_transport_channel_query_fn)0;
	xPort.pfQueryChannel = pfExpectedQuery;

	vAssertTrue(xSendRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "send request channel contract");
	vAssertTrue(xSendRequest.puPayload == auPayload, "send request payload pointer contract");
	vAssertTrue(xSendRequest.xPayloadLength == 3U, "send request payload length contract");
	vAssertTrue(xSendRequest.eReason == RSRX_REASON_CONNECT_REQUESTED, "send request reason contract");
	vAssertTrue(xFrame.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "frame channel contract");
	vAssertTrue(xFrame.puPayload == auPayload, "frame payload pointer contract");
	vAssertTrue(xFrame.xPayloadLength == sizeof(auPayload), "frame payload length contract");
	vAssertTrue(xFrame.eEventType == RSRX_TRANSPORT_EVENT_FRAME_RECEIVED, "frame event contract");
	vAssertTrue(xChannelState.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "channel state channel contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xChannelState.uIsAvailable == uExpectedAvailable, "channel state availability contract");
	vAssertTrue(xPort.pvContext == (void *)0, "transport port context contract");
	vAssertTrue(xPort.pfSend == (rsrx_transport_send_fn)0, "transport port layout contract");
	vAssertTrue(xPort.pfReceive == (rsrx_transport_receive_fn)0, "transport receive layout contract");
	/* cppcheck-suppress knownConditionTrueFalse */
	vAssertTrue(xPort.pfQueryChannel == pfExpectedQuery, "transport query layout contract");

	(void)printf("rsrx_transport_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
