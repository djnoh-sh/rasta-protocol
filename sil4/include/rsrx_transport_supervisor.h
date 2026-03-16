#ifndef RSRX_TRANSPORT_SUPERVISOR_H
#define RSRX_TRANSPORT_SUPERVISOR_H

#include <stdint.h>

#include "rsrx_api.h"
#include "rsrx_codec.h"

typedef enum
{
	RSRX_SUPERVISOR_STATUS_OK = 0,
	RSRX_SUPERVISOR_STATUS_INVALID_ARGUMENT,
	RSRX_SUPERVISOR_STATUS_DECODE_FAILED,
	RSRX_SUPERVISOR_STATUS_SESSION_ERROR,
	RSRX_SUPERVISOR_STATUS_NO_FRAME,
	RSRX_SUPERVISOR_STATUS_CHANNEL_DOWN,
	RSRX_SUPERVISOR_STATUS_RECEIVE_ERROR,
	RSRX_SUPERVISOR_STATUS_IGNORED_EVENT
} rsrx_supervisor_status_t;

typedef struct
{
	rsrx_transport_channel_state_t xLastChannelState;
	rsrx_transport_frame_t xLastFrame;
	rsrx_decoded_message_t xLastMessage;
	const rsrx_orchestrator_report_t * pxLastReport;
	uint32_t uProcessedFrameCount;
	uint32_t uPollCount;
} rsrx_transport_supervisor_report_t;

typedef struct
{
	rsrx_session_t * pxSession;
	rsrx_codec_port_t xCodec;
	rsrx_transport_supervisor_report_t xLastReport;
	uint32_t uInitialized;
} rsrx_transport_supervisor_context_t;

rsrx_supervisor_status_t rsrx_transport_supervisor_init(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_session_t * pxSession,
	const rsrx_codec_port_t * pxCodec);

rsrx_supervisor_status_t rsrx_transport_supervisor_process_frame(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport);

rsrx_supervisor_status_t rsrx_transport_supervisor_poll_receive(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_supervisor_report_t ** ppxReport);

rsrx_supervisor_status_t rsrx_transport_supervisor_process_transport_event(
	rsrx_transport_supervisor_context_t * pxContext,
	const rsrx_transport_frame_t * pxFrame,
	const rsrx_transport_supervisor_report_t ** ppxReport);

rsrx_supervisor_status_t rsrx_transport_supervisor_process_timer_expiry(
	rsrx_transport_supervisor_context_t * pxContext,
	rsrx_timer_expiry_source_t eTimerSource,
	const rsrx_transport_supervisor_report_t ** ppxReport);

#endif
