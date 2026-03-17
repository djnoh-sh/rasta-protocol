#include <stdio.h>
#include <stdlib.h>

#include "rsrx_channel_manager.h"
#include "rsrx_config_validator.h"
#include "rsrx_codec.h"

static void vAssertTrue(int iCondition, const char * pcMessage)
{
	if(iCondition == 0)
	{
		(void)fprintf(stderr, "ASSERT FAILED: %s\n", pcMessage);
		exit(EXIT_FAILURE);
	}
}

static rsrx_transport_status_t eTransportSend(void * pvContext, const rsrx_transport_send_request_t * pxRequest)
{
	(void)pvContext;
	(void)pxRequest;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportReceive(void * pvContext, rsrx_transport_frame_t * pxFrame)
{
	(void)pvContext;
	(void)pxFrame;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t eTransportQuery(void * pvContext, rsrx_transport_channel_state_t * pxState)
{
	(void)pvContext;
	(void)pxState;
	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_platform_status_t eClockNow(void * pvContext, rsrx_monotonic_time_ns_t * puNowNs)
{
	(void)pvContext;
	*puNowNs = 100U;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eTimerCommand(void * pvContext, const rsrx_timer_command_t * pxCommand)
{
	(void)pvContext;
	(void)pxCommand;
	return RSRX_PLATFORM_STATUS_OK;
}

static rsrx_platform_status_t eDiagnosticWrite(void * pvContext, const rsrx_diagnostic_record_t * pxRecord)
{
	(void)pvContext;
	(void)pxRecord;
	return RSRX_PLATFORM_STATUS_OK;
}

static void vApiNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport)
{
	(void)pvContext;
	(void)pxReport;
}

static void vApplicationDataNotify(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication)
{
	(void)pvContext;
	(void)pxReport;
	(void)pxIndication;
}

static void vLifecycleNotify(void * pvContext, const rsrx_orchestrator_report_t * pxReport, rsrx_action_t eAction, uint32_t uActionIndex)
{
	(void)pvContext;
	(void)pxReport;
	(void)eAction;
	(void)uActionIndex;
}

static void vFillValidConfig(rsrx_session_config_t * pxConfig, void * pvContext)
{
	static const uint8_t auPayload[2] = { 0x01U, 0x02U };

	pxConfig->xTransportPort.pvContext = pvContext;
	pxConfig->xTransportPort.pfSend = eTransportSend;
	pxConfig->xTransportPort.pfReceive = eTransportReceive;
	pxConfig->xTransportPort.pfQueryChannel = eTransportQuery;
	pxConfig->xCodecPort = *rsrx_codec_get_default_port();
	pxConfig->xPlatformPorts.xClock.pvContext = pvContext;
	pxConfig->xPlatformPorts.xClock.pfNow = eClockNow;
	pxConfig->xPlatformPorts.xTimer.pvContext = pvContext;
	pxConfig->xPlatformPorts.xTimer.pfCommand = eTimerCommand;
	pxConfig->xPlatformPorts.xDiagnostics.pvContext = pvContext;
	pxConfig->xPlatformPorts.xDiagnostics.pfWrite = eDiagnosticWrite;
	pxConfig->eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.eMode = RSRX_REDUNDANCY_MODE_SINGLE;
	pxConfig->xChannelManagerConfig.uChannelCount = 1U;
	pxConfig->xChannelManagerConfig.uPreferredChannelIndex = 0U;
	pxConfig->xChannelManagerConfig.uPreferredRecoveryHoldoffSelections = 0U;
	pxConfig->xChannelManagerConfig.axChannels[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
	pxConfig->xChannelManagerConfig.axChannels[0].uPriority = 0U;
	pxConfig->xChannelManagerConfig.axChannels[1].eChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	pxConfig->xChannelManagerConfig.axChannels[1].uIsAvailable = 0U;
	pxConfig->xChannelManagerConfig.axChannels[1].uPriority = 0U;
	pxConfig->puFramePayload = auPayload;
	pxConfig->xFramePayloadLength = sizeof(auPayload);
	pxConfig->uSupervisionIntervalNs = 100U;
	pxConfig->uRetransmissionIntervalNs = 200U;
	pxConfig->uDiagnosticFlushIntervalNs = 300U;
	pxConfig->pvApplicationDataContext = pvContext;
	pxConfig->pfApplicationData = vApplicationDataNotify;
	pxConfig->pvApiCallbackContext = pvContext;
	pxConfig->pfApiNotification = vApiNotify;
	pxConfig->pvLifecycleCallbackContext = pvContext;
	pxConfig->pfLifecycleNotification = vLifecycleNotify;
}

static void vTestValidConfiguration(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_OK, "valid config");
	vAssertTrue(xReport.eStatus == RSRX_CONFIG_STATUS_OK, "valid report status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_NONE, "valid report field");
}

static void vTestMissingTransportPort(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	xConfig.xTransportPort.pfSend = (rsrx_transport_send_fn)0;

	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD, "missing transport status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_TRANSPORT_PORT, "missing transport field");
}

static void vTestMissingCodecPort(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	xConfig.xCodecPort.pfEncode = (rsrx_encode_message_fn)0;

	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD, "missing codec status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_CODEC_PORT, "missing codec field");
}

static void vTestInvalidIntervals(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	xConfig.uSupervisionIntervalNs = 0U;

	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_INVALID_RANGE, "invalid interval status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_SUPERVISION_INTERVAL, "invalid interval field");
}

static void vTestMissingApplicationCallback(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	xConfig.pfApplicationData = (rsrx_application_data_fn)0;

	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD, "missing application callback status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_APPLICATION_DATA_CALLBACK, "missing application callback field");
}

static void vTestInconsistentPayload(void)
{
	rsrx_session_config_t xConfig;
	rsrx_config_validation_report_t xReport;
	uint32_t uContext = 0U;

	vFillValidConfig(&xConfig, &uContext);
	xConfig.puFramePayload = (const uint8_t *)0;
	xConfig.xFramePayloadLength = 4U;

	vAssertTrue(rsrx_validate_session_config(&xConfig, &xReport) == RSRX_CONFIG_STATUS_INCONSISTENT_VALUE, "inconsistent payload status");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_FRAME_PAYLOAD, "inconsistent payload field");
}

static void vTestInvalidArguments(void)
{
	rsrx_config_validation_report_t xReport;

	vAssertTrue(rsrx_validate_session_config((const rsrx_session_config_t *)0, &xReport) == RSRX_CONFIG_STATUS_INVALID_ARGUMENT, "null config");
	vAssertTrue(xReport.eField == RSRX_CONFIG_FIELD_NONE, "null config field");
}

int main(void)
{
	vTestValidConfiguration();
	vTestMissingTransportPort();
	vTestMissingCodecPort();
	vTestInvalidIntervals();
	vTestMissingApplicationCallback();
	vTestInconsistentPayload();
	vTestInvalidArguments();

	(void)printf("rsrx_config_validator_test: all tests passed\n");
	return EXIT_SUCCESS;
}
