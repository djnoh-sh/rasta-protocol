#ifndef RSRX_PROTOCOL_CONTEXT_H
#define RSRX_PROTOCOL_CONTEXT_H

#include "rsrx_codec.h"

typedef struct
{
	uint32_t uNextTxSequenceNumber;
	uint32_t uLastRxSequenceNumber;
	uint32_t uLastTxConfirmationNumber;
	uint32_t uLastRemoteConfirmationNumber;
	uint32_t uLastRetransmissionRequestTxSequenceNumber;
	uint32_t uRetransmissionBaseSequenceNumber;
	uint32_t uRetransmissionPending;
} rsrx_protocol_context_t;

rsrx_status_t rsrx_protocol_context_init(
	rsrx_protocol_context_t * pxContext);

rsrx_status_t rsrx_protocol_context_record_inbound_message(
	rsrx_protocol_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage);

rsrx_status_t rsrx_protocol_context_resolve_inbound_event(
	const rsrx_protocol_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage,
	rsrx_event_t * peEvent);

rsrx_status_t rsrx_protocol_context_build_encode_request(
	rsrx_protocol_context_t * pxContext,
	rsrx_message_type_t eMessageType,
	rsrx_reason_code_t eReason,
	const uint8_t * puPayload,
	size_t xPayloadLength,
	rsrx_encode_request_t * pxRequest);

rsrx_status_t rsrx_protocol_context_clear_retransmission(
	rsrx_protocol_context_t * pxContext);

#endif
