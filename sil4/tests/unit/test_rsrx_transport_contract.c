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
	xChannelState.uIsAvailable = 1U;

	xPort.pvContext = (void *)0;
	xPort.pfSend = (rsrx_transport_send_fn)0;
	xPort.pfReceive = (rsrx_transport_receive_fn)0;
	xPort.pfQueryChannel = (rsrx_transport_channel_query_fn)0;

	vAssertTrue(xSendRequest.eChannelId == RSRX_TRANSPORT_CHANNEL_PRIMARY, "send request channel contract");
	vAssertTrue(xSendRequest.xPayloadLength == 3U, "send request payload length contract");
	vAssertTrue(xFrame.eEventType == RSRX_TRANSPORT_EVENT_FRAME_RECEIVED, "frame event contract");
	vAssertTrue(xChannelState.uIsAvailable == 1U, "channel state availability contract");
	vAssertTrue(xPort.pfSend == (rsrx_transport_send_fn)0, "transport port layout contract");

	(void)printf("rsrx_transport_contract_test: all tests passed\n");

	return EXIT_SUCCESS;
}
