#ifndef RSRX_PLATFORM_ADAPTERS_H
#define RSRX_PLATFORM_ADAPTERS_H

#include <stdint.h>

#include "rsrx_codec.h"
#include "rsrx_orchestrator.h"
#include "rsrx_platform.h"
#include "rsrx_protocol_context.h"
#include "rsrx_transport.h"

typedef struct
{
	rsrx_transport_port_t xTransportPort;
	rsrx_codec_port_t xCodecPort;
	rsrx_protocol_context_t xProtocolContext;
	rsrx_decoded_message_t xLastInboundMessage;
	rsrx_transport_channel_id_t eDefaultChannelId;
	const uint8_t * puFramePayload;
	size_t xFramePayloadLength;
	uint8_t auEncodedFrame[D_RSRX_CODEC_MAX_FRAME_BYTES];
	uint32_t uHasLastInboundMessage;
} rsrx_transport_adapter_context_t;

typedef struct
{
	rsrx_platform_port_table_t xPlatformPorts;
	rsrx_monotonic_time_ns_t uSupervisionIntervalNs;
	rsrx_monotonic_time_ns_t uRetransmissionIntervalNs;
	rsrx_monotonic_time_ns_t uDiagnosticFlushIntervalNs;
	uint32_t uEventCounter;
} rsrx_platform_adapter_context_t;

rsrx_transport_status_t rsrx_transport_adapter_init(
	rsrx_transport_adapter_context_t * pxContext,
	const rsrx_transport_port_t * pxTransportPort,
	const rsrx_codec_port_t * pxCodecPort,
	rsrx_transport_channel_id_t eDefaultChannelId,
	const uint8_t * puFramePayload,
	size_t xFramePayloadLength);

void rsrx_transport_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex);

void rsrx_transport_adapter_record_inbound_message(
	rsrx_transport_adapter_context_t * pxContext,
	const rsrx_decoded_message_t * pxMessage);

const rsrx_decoded_message_t * rsrx_transport_adapter_get_last_inbound_message(
	const rsrx_transport_adapter_context_t * pxContext);

rsrx_transport_status_t rsrx_transport_adapter_query_channel(
	const rsrx_transport_adapter_context_t * pxContext,
	rsrx_transport_channel_state_t * pxState);

rsrx_transport_status_t rsrx_transport_adapter_receive_frame(
	const rsrx_transport_adapter_context_t * pxContext,
	rsrx_transport_frame_t * pxFrame);

rsrx_transport_status_t rsrx_transport_adapter_send_application_data(
	rsrx_transport_adapter_context_t * pxContext,
	const uint8_t * puPayload,
	size_t xPayloadLength);

void rsrx_transport_adapter_clear_retransmission_context(
	rsrx_transport_adapter_context_t * pxContext);

rsrx_platform_status_t rsrx_platform_adapter_init(
	rsrx_platform_adapter_context_t * pxContext,
	const rsrx_platform_port_table_t * pxPorts,
	rsrx_monotonic_time_ns_t uSupervisionIntervalNs,
	rsrx_monotonic_time_ns_t uRetransmissionIntervalNs,
	rsrx_monotonic_time_ns_t uDiagnosticFlushIntervalNs);

void rsrx_platform_timer_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex);

void rsrx_platform_diagnostics_executor_dispatch(
	void * pvContext,
	const rsrx_transition_result_t * pxTransition,
	rsrx_action_t eAction,
	uint32_t uActionIndex);

rsrx_status_t rsrx_platform_adapter_build_executor_table(
	rsrx_action_executor_table_t * pxExecutors,
	rsrx_transport_adapter_context_t * pxTransportContext,
	rsrx_platform_adapter_context_t * pxPlatformContext,
	const rsrx_action_executor_t * pxApplicationExecutor,
	const rsrx_action_executor_t * pxApiExecutor,
	const rsrx_action_executor_t * pxLifecycleExecutor);

#endif
