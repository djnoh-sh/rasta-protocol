#ifndef RSRX_API_H
#define RSRX_API_H

#include <stddef.h>
#include <stdint.h>

#include "rsrx_channel_manager.h"
#include "rsrx_orchestrator.h"
#include "rsrx_platform_adapters.h"

typedef struct rsrx_session rsrx_session_t;

typedef struct
{
	const uint8_t * puPayload;
	size_t xPayloadLength;
	rsrx_reason_code_t eReason;
	uint32_t uSequenceNumber;
	uint32_t uConfirmationNumber;
} rsrx_application_data_indication_t;

typedef void (*rsrx_api_notification_fn)(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport);

typedef void (*rsrx_application_data_fn)(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication);

typedef void (*rsrx_lifecycle_notification_fn)(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	rsrx_action_t eAction,
	uint32_t uActionIndex);

typedef enum
{
	RSRX_TIMER_EXPIRY_INVALID = 0,
	RSRX_TIMER_EXPIRY_SUPERVISION,
	RSRX_TIMER_EXPIRY_RETRANSMISSION,
	RSRX_TIMER_EXPIRY_DIAGNOSTIC_FLUSH
} rsrx_timer_expiry_source_t;

typedef struct
{
	rsrx_transport_port_t xTransportPort;
	rsrx_codec_port_t xCodecPort;
	rsrx_platform_port_table_t xPlatformPorts;
	rsrx_transport_channel_id_t eDefaultChannelId;
	rsrx_channel_manager_config_t xChannelManagerConfig;
	const uint8_t * puFramePayload;
	size_t xFramePayloadLength;
	rsrx_monotonic_time_ns_t uSupervisionIntervalNs;
	rsrx_monotonic_time_ns_t uRetransmissionIntervalNs;
	rsrx_monotonic_time_ns_t uDiagnosticFlushIntervalNs;
	void * pvApplicationDataContext;
	rsrx_application_data_fn pfApplicationData;
	void * pvApiCallbackContext;
	rsrx_api_notification_fn pfApiNotification;
	void * pvLifecycleCallbackContext;
	rsrx_lifecycle_notification_fn pfLifecycleNotification;
} rsrx_session_config_t;

struct rsrx_session
{
	rsrx_transport_adapter_context_t xTransportAdapter;
	rsrx_channel_manager_context_t xChannelManager;
	rsrx_platform_adapter_context_t xPlatformAdapter;
	rsrx_orchestrator_context_t xOrchestrator;
	rsrx_action_executor_table_t xExecutors;
	rsrx_orchestrator_report_t xLastReport;
	void * pvApplicationDataContext;
	rsrx_application_data_fn pfApplicationData;
	void * pvApiCallbackContext;
	rsrx_api_notification_fn pfApiNotification;
	void * pvLifecycleCallbackContext;
	rsrx_lifecycle_notification_fn pfLifecycleNotification;
	uint32_t uInitialized;
};

rsrx_status_t rsrx_session_init(
	rsrx_session_t * pxSession,
	const rsrx_session_config_t * pxConfig);

rsrx_status_t rsrx_session_start(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport);

rsrx_status_t rsrx_session_connect(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport);

rsrx_status_t rsrx_session_disconnect(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport);

rsrx_status_t rsrx_session_process_event(
	rsrx_session_t * pxSession,
	rsrx_event_t eEvent,
	const rsrx_orchestrator_report_t ** ppxReport);

rsrx_status_t rsrx_session_process_timer_expiry(
	rsrx_session_t * pxSession,
	rsrx_timer_expiry_source_t eTimerSource,
	const rsrx_orchestrator_report_t ** ppxReport);

rsrx_status_t rsrx_session_send_application_data(
	rsrx_session_t * pxSession,
	const uint8_t * puPayload,
	size_t xPayloadLength);

rsrx_state_t rsrx_session_get_state(
	const rsrx_session_t * pxSession);

rsrx_status_t rsrx_session_reset(
	rsrx_session_t * pxSession);

#endif
