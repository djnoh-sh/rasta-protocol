#include "rsrx_protocol_context.h"

static void vWriteUint32BigEndian(
	uint8_t * puBuffer,
	uint32_t uValue)
{
	puBuffer[0] = (uint8_t)((uValue >> 24) & 0xFFU);
	puBuffer[1] = (uint8_t)((uValue >> 16) & 0xFFU);
	puBuffer[2] = (uint8_t)((uValue >> 8) & 0xFFU);
	puBuffer[3] = (uint8_t)(uValue & 0xFFU);
}

rsrx_status_t rsrx_protocol_context_init(
	rsrx_protocol_context_t * pxContext)
{
	if(pxContext == (rsrx_protocol_context_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uNextTxSequenceNumber = 1U;
	pxContext->uLastRxSequenceNumber = 0U;
	pxContext->uLastTxConfirmationNumber = 0U;
	pxContext->uRetransmissionBaseSequenceNumber = 0U;
	pxContext->uRetransmissionPending = 0U;

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_protocol_context_record_inbound_message(
	rsrx_protocol_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage)
{
	if((pxContext == (rsrx_protocol_context_t *)0) ||
		(pxMessage == (const rsrx_decoded_message_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	if(pxMessage->uSequenceNumber > pxContext->uLastRxSequenceNumber)
	{
		pxContext->uLastRxSequenceNumber = pxMessage->uSequenceNumber;
		pxContext->uLastTxConfirmationNumber = pxMessage->uSequenceNumber;
	}

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_protocol_context_resolve_inbound_event(
	const rsrx_protocol_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage,
	rsrx_event_t * peEvent)
{
	if((pxContext == (const rsrx_protocol_context_t *)0) ||
		(pxMessage == (const rsrx_decoded_message_t *)0) ||
		(peEvent == (rsrx_event_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	switch(pxMessage->eMessageType)
	{
		case RSRX_MESSAGE_TYPE_CONNECT_REQUEST:
		case RSRX_MESSAGE_TYPE_DISCONNECT:
		case RSRX_MESSAGE_TYPE_DIAGNOSTIC:
			*peEvent = pxMessage->eSuggestedEvent;
			return RSRX_STATUS_OK;

		case RSRX_MESSAGE_TYPE_CONNECT_RESPONSE:
		case RSRX_MESSAGE_TYPE_HEARTBEAT:
		case RSRX_MESSAGE_TYPE_DATA:
		case RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST:
			if(pxContext->uLastRxSequenceNumber == 0U)
			{
				*peEvent = pxMessage->eSuggestedEvent;
				return RSRX_STATUS_OK;
			}

			if(pxMessage->uSequenceNumber == (pxContext->uLastRxSequenceNumber + 1U))
			{
				*peEvent = pxMessage->eSuggestedEvent;
				return RSRX_STATUS_OK;
			}

			if(pxMessage->uSequenceNumber > (pxContext->uLastRxSequenceNumber + 1U))
			{
				*peEvent = RSRX_EVENT_SEQUENCE_GAP_DETECTED;
				return RSRX_STATUS_OK;
			}

			*peEvent = RSRX_EVENT_PROTOCOL_ERROR;
			return RSRX_STATUS_OK;

		case RSRX_MESSAGE_TYPE_INVALID:
		default:
			return RSRX_STATUS_INVALID_ARGUMENT;
	}
}

rsrx_status_t rsrx_protocol_context_build_encode_request(
	rsrx_protocol_context_t * pxContext,
	rsrx_message_type_t eMessageType,
	rsrx_reason_code_t eReason,
	const uint8_t * puPayload,
	size_t xPayloadLength,
	rsrx_encode_request_t * pxRequest)
{
	static uint8_t auRetransmissionPayload[4];

	if((pxContext == (rsrx_protocol_context_t *)0) ||
		(pxRequest == (rsrx_encode_request_t *)0))
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxRequest->eMessageType = eMessageType;
	pxRequest->eReason = eReason;
	pxRequest->uSequenceNumber = pxContext->uNextTxSequenceNumber;
	pxRequest->uConfirmationNumber = pxContext->uLastTxConfirmationNumber;
	pxRequest->puPayload = puPayload;
	pxRequest->xPayloadLength = xPayloadLength;

	if(eMessageType == RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST)
	{
		if(pxContext->uRetransmissionPending == 0U)
		{
			pxContext->uRetransmissionBaseSequenceNumber =
				pxContext->uLastRxSequenceNumber + 1U;
			pxContext->uRetransmissionPending = 1U;
		}

		vWriteUint32BigEndian(
			auRetransmissionPayload,
			pxContext->uRetransmissionBaseSequenceNumber);
		pxRequest->puPayload = auRetransmissionPayload;
		pxRequest->xPayloadLength = sizeof(auRetransmissionPayload);
	}

	pxContext->uNextTxSequenceNumber++;

	return RSRX_STATUS_OK;
}

rsrx_status_t rsrx_protocol_context_clear_retransmission(
	rsrx_protocol_context_t * pxContext)
{
	if(pxContext == (rsrx_protocol_context_t *)0)
	{
		return RSRX_STATUS_INVALID_ARGUMENT;
	}

	pxContext->uRetransmissionBaseSequenceNumber = 0U;
	pxContext->uRetransmissionPending = 0U;
	return RSRX_STATUS_OK;
}
