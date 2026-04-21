#include "rsrx_config_validator.h"

static void vSetReport(
	rsrx_config_validation_report_t * pxReport,
	rsrx_config_status_t eStatus,
	rsrx_config_field_t eField);

static uint32_t uChannelManagerConfigIsValid(
	const rsrx_session_config_t * pxConfig,
	rsrx_config_validation_report_t * pxReport)
{
	rsrx_channel_manager_context_t xContext;

	if(rsrx_channel_manager_init(&xContext, &pxConfig->xChannelManagerConfig) !=
		RSRX_CHANNEL_MANAGER_STATUS_OK)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_RANGE,
			RSRX_CONFIG_FIELD_DEFAULT_CHANNEL);
		return 0U;
	}

	return 1U;
}

static uint32_t uDefaultChannelBelongsToConfiguredTopology(
	const rsrx_session_config_t * pxConfig,
	rsrx_config_validation_report_t * pxReport)
{
	uint32_t uIndex;
	uint32_t uFound;

	uFound = 0U;
	for(uIndex = 0U; uIndex < pxConfig->xChannelManagerConfig.uChannelCount; ++uIndex)
	{
		if(pxConfig->xChannelManagerConfig.axChannels[uIndex].eChannelId ==
			pxConfig->eDefaultChannelId)
		{
			uFound = 1U;
		}
	}

	if(uFound == 0U)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INCONSISTENT_VALUE,
			RSRX_CONFIG_FIELD_DEFAULT_CHANNEL);
		return 0U;
	}

	return 1U;
}

static void vSetReport(
	rsrx_config_validation_report_t * pxReport,
	rsrx_config_status_t eStatus,
	rsrx_config_field_t eField)
{
	if(pxReport != (rsrx_config_validation_report_t *)0)
	{
		pxReport->eStatus = eStatus;
		pxReport->eField = eField;
	}
}

static uint32_t uPayloadIsConsistent(
	const rsrx_session_config_t * pxConfig,
	rsrx_config_validation_report_t * pxReport)
{
	if((pxConfig->puFramePayload == (const uint8_t *)0) &&
		(pxConfig->xFramePayloadLength != 0U))
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INCONSISTENT_VALUE,
			RSRX_CONFIG_FIELD_FRAME_PAYLOAD);
		return 0U;
	}

	return 1U;
}

static uint32_t uIntervalsAreValid(
	const rsrx_session_config_t * pxConfig,
	rsrx_config_validation_report_t * pxReport)
{
	if(pxConfig->uSupervisionIntervalNs == 0U)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_RANGE,
			RSRX_CONFIG_FIELD_SUPERVISION_INTERVAL);
		return 0U;
	}

	if(pxConfig->uRetransmissionIntervalNs == 0U)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_RANGE,
			RSRX_CONFIG_FIELD_RETRANSMISSION_INTERVAL);
		return 0U;
	}

	if(pxConfig->uDiagnosticFlushIntervalNs == 0U)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_RANGE,
			RSRX_CONFIG_FIELD_DIAGNOSTIC_FLUSH_INTERVAL);
		return 0U;
	}

	return 1U;
}

rsrx_config_status_t rsrx_validate_session_config(
	const rsrx_session_config_t * pxConfig,
	rsrx_config_validation_report_t * pxReport)
{
	if(pxConfig == (const rsrx_session_config_t *)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_ARGUMENT,
			RSRX_CONFIG_FIELD_NONE);
		return RSRX_CONFIG_STATUS_INVALID_ARGUMENT;
	}

	if(pxConfig->xTransportPort.pfSend == (rsrx_transport_send_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_TRANSPORT_PORT);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xTransportPort.pfReceive == (rsrx_transport_receive_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_TRANSPORT_PORT);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xTransportPort.pfQueryChannel == (rsrx_transport_channel_query_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_TRANSPORT_PORT);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xCodecPort.pfEncode == (rsrx_encode_message_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_CODEC_PORT);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xPlatformPorts.xClock.pfNow == (rsrx_clock_now_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_PLATFORM_CLOCK);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xPlatformPorts.xTimer.pfCommand == (rsrx_timer_command_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_PLATFORM_TIMER);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->xPlatformPorts.xDiagnostics.pfWrite == (rsrx_diagnostic_write_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_PLATFORM_DIAGNOSTICS);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->pfApplicationData == (rsrx_application_data_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_APPLICATION_DATA_CALLBACK);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->pfApiNotification == (rsrx_api_notification_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_API_CALLBACK);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->pfLifecycleNotification == (rsrx_lifecycle_notification_fn)0)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD,
			RSRX_CONFIG_FIELD_LIFECYCLE_CALLBACK);
		return RSRX_CONFIG_STATUS_MISSING_REQUIRED_FIELD;
	}

	if(pxConfig->eDefaultChannelId == RSRX_TRANSPORT_CHANNEL_INVALID)
	{
		vSetReport(
			pxReport,
			RSRX_CONFIG_STATUS_INVALID_RANGE,
			RSRX_CONFIG_FIELD_DEFAULT_CHANNEL);
		return RSRX_CONFIG_STATUS_INVALID_RANGE;
	}

	if(uPayloadIsConsistent(pxConfig, pxReport) == 0U)
	{
		return pxReport->eStatus;
	}

	if(uChannelManagerConfigIsValid(pxConfig, pxReport) == 0U)
	{
		return pxReport->eStatus;
	}

	if(uDefaultChannelBelongsToConfiguredTopology(pxConfig, pxReport) == 0U)
	{
		return pxReport->eStatus;
	}

	if(uIntervalsAreValid(pxConfig, pxReport) == 0U)
	{
		return pxReport->eStatus;
	}

	vSetReport(
		pxReport,
		RSRX_CONFIG_STATUS_OK,
		RSRX_CONFIG_FIELD_NONE);
	return RSRX_CONFIG_STATUS_OK;
}
