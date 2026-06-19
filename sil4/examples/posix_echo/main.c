#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "rsrx_api.h"
#include "rsrx_transport_supervisor.h"
#include "rsrx_codec.h"
#include "posix_platform.h"
#include "posix_transport.h"

#define SERVER_PORT 8888
#define CLIENT_PORT 8889
#define SERVER_SECONDARY_PORT 8890
#define CLIENT_SECONDARY_PORT 8891
#define DEFAULT_SEND_INTERVAL_MS 2000U
#define DEFAULT_RUN_DURATION_SEC 0U
#define DEFAULT_HOLDOFF_SELECTIONS 2U

typedef struct
{
	const char * pcRole;
	const char * pcRemoteIp;
	const char * pcMessage;
	uint16_t uLocalPort;
	uint16_t uRemotePort;
	uint16_t uLocalPortSecondary;
	uint16_t uRemotePortSecondary;
	uint32_t uSendIntervalMs;
	uint32_t uRunDurationSec;
	uint32_t uEnableRedundant;
	uint32_t uPreferredRecoveryHoldoffSelections;
	uint32_t uPrimaryDownAtSec;
	uint32_t uPrimaryUpAtSec;
	uint32_t uPrimaryDownAfterReceiveCount;
	uint32_t uIsServer;
} app_options_t;

typedef struct
{
	rsrx_session_t * pxSession;
	uint32_t uIsServer;
	uint32_t uSendCount;
	uint32_t uReceiveCount;
	rsrx_transport_channel_id_t eLastObservedChannelId;
	uint32_t uLastObservedSwitchCount;
} app_context_t;

/* This example is intentionally small and linear:
 * 1. parse CLI options
 * 2. build transport/platform ports
 * 3. assemble rsrx_session_config_t
 * 4. start session and optional client connect
 * 5. run select/timer loop
 */

static volatile sig_atomic_t g_iKeepRunning = 1;

static void vHandleSignal(
	int iSignal)
{
	(void)iSignal;
	g_iKeepRunning = 0;
}

static void vPrintUsage(
	const char * pcProgramName)
{
	(void)printf(
		"Usage: %s [server|client] [--local-port N] [--remote-port N] [--remote-ip IP] [--message TEXT] [--send-interval-ms N] [--duration-sec N] [--redundant 0|1] [--local-port-secondary N] [--remote-port-secondary N] [--holdoff-selections N] [--primary-down-at-sec N] [--primary-up-at-sec N] [--primary-down-after-rx N]\n",
		pcProgramName);
}

static int iParseUint16(
	const char * pcText,
	uint16_t * puValue)
{
	char * pcEnd;
	unsigned long ulValue;

	if((pcText == (const char *)0) || (puValue == (uint16_t *)0))
	{
		return -1;
	}

	ulValue = strtoul(pcText, &pcEnd, 10);
	if((*pcText == '\0') || (*pcEnd != '\0') || (ulValue > 65535UL))
	{
		return -1;
	}

	*puValue = (uint16_t)ulValue;
	return 0;
}

static int iParseUint32(
	const char * pcText,
	uint32_t * puValue)
{
	char * pcEnd;
	unsigned long ulValue;

	if((pcText == (const char *)0) || (puValue == (uint32_t *)0))
	{
		return -1;
	}

	ulValue = strtoul(pcText, &pcEnd, 10);
	if((*pcText == '\0') || (*pcEnd != '\0') || (ulValue > 0xFFFFFFFFUL))
	{
		return -1;
	}

	*puValue = (uint32_t)ulValue;
	return 0;
}

static int iParseArguments(
	int argc,
	char * argv[],
	app_options_t * pxOptions)
{
	int iIndex;

	if((argc < 2) || (pxOptions == (app_options_t *)0))
	{
		return -1;
	}

	(void)memset(pxOptions, 0, sizeof(*pxOptions));
	pxOptions->pcRole = argv[1];
	pxOptions->pcRemoteIp = "127.0.0.1";
	pxOptions->pcMessage = "Hello Server!";
	pxOptions->uSendIntervalMs = DEFAULT_SEND_INTERVAL_MS;
	pxOptions->uRunDurationSec = DEFAULT_RUN_DURATION_SEC;
	pxOptions->uPreferredRecoveryHoldoffSelections = DEFAULT_HOLDOFF_SELECTIONS;

	if(strcmp(argv[1], "server") == 0)
	{
		pxOptions->uIsServer = 1U;
		pxOptions->uLocalPort = SERVER_PORT;
		pxOptions->uRemotePort = CLIENT_PORT;
		pxOptions->uLocalPortSecondary = SERVER_SECONDARY_PORT;
		pxOptions->uRemotePortSecondary = CLIENT_SECONDARY_PORT;
	}
	else if(strcmp(argv[1], "client") == 0)
	{
		pxOptions->uIsServer = 0U;
		pxOptions->uLocalPort = CLIENT_PORT;
		pxOptions->uRemotePort = SERVER_PORT;
		pxOptions->uLocalPortSecondary = CLIENT_SECONDARY_PORT;
		pxOptions->uRemotePortSecondary = SERVER_SECONDARY_PORT;
	}
	else
	{
		return -1;
	}

	for(iIndex = 2; iIndex < argc; iIndex += 2)
	{
		if((iIndex + 1) >= argc)
		{
			return -1;
		}

		if(strcmp(argv[iIndex], "--local-port") == 0)
		{
			if(iParseUint16(argv[iIndex + 1], &pxOptions->uLocalPort) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--remote-port") == 0)
		{
			if(iParseUint16(argv[iIndex + 1], &pxOptions->uRemotePort) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--remote-ip") == 0)
		{
			pxOptions->pcRemoteIp = argv[iIndex + 1];
		}
		else if(strcmp(argv[iIndex], "--message") == 0)
		{
			pxOptions->pcMessage = argv[iIndex + 1];
		}
		else if(strcmp(argv[iIndex], "--send-interval-ms") == 0)
		{
			if(iParseUint32(argv[iIndex + 1], &pxOptions->uSendIntervalMs) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--duration-sec") == 0)
		{
			if(iParseUint32(argv[iIndex + 1], &pxOptions->uRunDurationSec) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--redundant") == 0)
		{
			if(iParseUint32(argv[iIndex + 1], &pxOptions->uEnableRedundant) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--local-port-secondary") == 0)
		{
			if(iParseUint16(argv[iIndex + 1], &pxOptions->uLocalPortSecondary) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--remote-port-secondary") == 0)
		{
			if(iParseUint16(argv[iIndex + 1], &pxOptions->uRemotePortSecondary) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--holdoff-selections") == 0)
		{
			if(iParseUint32(
				argv[iIndex + 1],
				&pxOptions->uPreferredRecoveryHoldoffSelections) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--primary-down-at-sec") == 0)
		{
			if(iParseUint32(argv[iIndex + 1], &pxOptions->uPrimaryDownAtSec) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--primary-up-at-sec") == 0)
		{
			if(iParseUint32(argv[iIndex + 1], &pxOptions->uPrimaryUpAtSec) != 0)
			{
				return -1;
			}
		}
		else if(strcmp(argv[iIndex], "--primary-down-after-rx") == 0)
		{
			if(iParseUint32(
				argv[iIndex + 1],
				&pxOptions->uPrimaryDownAfterReceiveCount) != 0)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

static const char * pcChannelName(
	rsrx_transport_channel_id_t eChannelId)
{
	switch(eChannelId)
	{
		case RSRX_TRANSPORT_CHANNEL_PRIMARY:
			return "primary";

		case RSRX_TRANSPORT_CHANNEL_SECONDARY:
			return "secondary";

		case RSRX_TRANSPORT_CHANNEL_REDUNDANT:
			return "redundant";

		case RSRX_TRANSPORT_CHANNEL_INVALID:
		default:
			return "invalid";
	}
}

static void vLogWithTimestamp(
	const char * pcPrefix,
	const char * pcMessage)
{
	struct timespec xTimestamp;
	long long iMilliseconds;

	(void)clock_gettime(CLOCK_MONOTONIC, &xTimestamp);
	iMilliseconds = ((long long)xTimestamp.tv_sec * 1000LL) +
		((long long)xTimestamp.tv_nsec / 1000000LL);
	(void)printf("[%lld ms] %s%s\n", iMilliseconds, pcPrefix, pcMessage);
}

static void on_api_notification(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport)
{
	app_context_t * pxContext = (app_context_t *)pvContext;

	(void)pxContext;
	if((pxReport != (const rsrx_orchestrator_report_t *)0) &&
		(pxReport->xTransition.eStatus != RSRX_STATUS_OK))
	{
		(void)printf(
			"API notification: status=%u reason=0x%04X\n",
			(unsigned int)pxReport->xTransition.eStatus,
			(unsigned int)pxReport->xTransition.eReason);
	}
}

static void on_application_data(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	const rsrx_application_data_indication_t * pxIndication)
{
	app_context_t * pxContext = (app_context_t *)pvContext;
	rsrx_status_t eStatus;

	(void)pxReport;
	if((pxContext == (app_context_t *)0) ||
		(pxIndication == (const rsrx_application_data_indication_t *)0))
	{
		return;
	}

	pxContext->uReceiveCount++;
	(void)printf(
		"RX[%u]: %.*s\n",
		(unsigned int)pxContext->uReceiveCount,
		(int)pxIndication->xPayloadLength,
		(const char *)pxIndication->puPayload);

	if(pxContext->uIsServer != 0U)
	{
		eStatus = rsrx_session_send_application_data(
			pxContext->pxSession,
			pxIndication->puPayload,
			pxIndication->xPayloadLength);
		(void)printf("Echo send status=%u\n", (unsigned int)eStatus);
	}
}

static void on_lifecycle_notification(
	void * pvContext,
	const rsrx_orchestrator_report_t * pxReport,
	rsrx_action_t eAction,
	uint32_t uActionIndex)
{
	(void)pvContext;
	(void)uActionIndex;
	if(pxReport == (const rsrx_orchestrator_report_t *)0)
	{
		return;
	}

	if(pxReport->xTransition.ePreviousState != pxReport->xTransition.eNextState)
	{
		(void)printf(
			"State change: %u -> %u by action %u\n",
			(unsigned int)pxReport->xTransition.ePreviousState,
			(unsigned int)pxReport->xTransition.eNextState,
			(unsigned int)eAction);
	}
}

static void vLogSupervisorReport(
	app_context_t * pxAppContext,
	const rsrx_transport_supervisor_report_t * pxSupervisorReport)
{
	if((pxAppContext == (app_context_t *)0) ||
		(pxSupervisorReport == (const rsrx_transport_supervisor_report_t *)0))
	{
		return;
	}

	if((pxSupervisorReport->uChannelSwitchCount !=
			pxAppContext->uLastObservedSwitchCount) ||
		(pxSupervisorReport->xLastChannelState.eChannelId !=
			pxAppContext->eLastObservedChannelId))
	{
		(void)printf(
			"Channel report: active=%s available=%u switch_count=%u failover=%u preferred_recovery=%u holdoff_active=%u remaining=%u\n",
			pcChannelName(pxSupervisorReport->xLastChannelState.eChannelId),
			(unsigned int)pxSupervisorReport->uAvailableChannelCount,
			(unsigned int)pxSupervisorReport->uChannelSwitchCount,
			(unsigned int)pxSupervisorReport->uFailoverSwitchCount,
			(unsigned int)pxSupervisorReport->uPreferredRecoverySwitchCount,
			(unsigned int)pxSupervisorReport->uPreferredRecoveryHoldoffActive,
			(unsigned int)pxSupervisorReport->uPreferredRecoveryHoldoffRemainingCount);
		pxAppContext->uLastObservedSwitchCount =
			pxSupervisorReport->uChannelSwitchCount;
		pxAppContext->eLastObservedChannelId =
			pxSupervisorReport->xLastChannelState.eChannelId;
	}
}

static void vProcessChannelToggle(
	posix_transport_context_t * pxTransportContext,
	rsrx_transport_supervisor_context_t * pxSupervisor,
	app_context_t * pxAppContext,
	rsrx_transport_channel_id_t eChannelId,
	rsrx_transport_event_type_t eEventType,
	const char * pcReason)
{
	rsrx_transport_frame_t xEventFrame;
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;

	if((pxTransportContext == (posix_transport_context_t *)0) ||
		(pxSupervisor == (rsrx_transport_supervisor_context_t *)0))
	{
		return;
	}

	if(posix_transport_set_channel_availability(
		pxTransportContext,
		eChannelId,
		(uint32_t)(eEventType == RSRX_TRANSPORT_EVENT_CHANNEL_UP)) != 0)
	{
		return;
	}

	(void)memset(&xEventFrame, 0, sizeof(xEventFrame));
	xEventFrame.eChannelId = eChannelId;
	xEventFrame.eEventType = eEventType;
	if(rsrx_transport_supervisor_process_transport_event(
		pxSupervisor,
		&xEventFrame,
		&pxSupervisorReport) == RSRX_SUPERVISOR_STATUS_IGNORED_EVENT)
	{
		(void)printf(
			"Injected transport event: %s %s\n",
			pcChannelName(eChannelId),
			pcReason);
		vLogSupervisorReport(pxAppContext, pxSupervisorReport);
	}
}

static void vBuildTransportEndpoints(
	const app_options_t * pxOptions,
	posix_transport_endpoint_t axEndpoints[MAX_CHANNELS])
{
	(void)memset(axEndpoints, 0, sizeof(posix_transport_endpoint_t) * MAX_CHANNELS);

	axEndpoints[0].eChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	axEndpoints[0].uLocalPort = pxOptions->uLocalPort;
	axEndpoints[0].pcRemoteIp = pxOptions->pcRemoteIp;
	axEndpoints[0].uRemotePort = pxOptions->uRemotePort;

	axEndpoints[1].eChannelId = RSRX_TRANSPORT_CHANNEL_SECONDARY;
	axEndpoints[1].uLocalPort = pxOptions->uLocalPortSecondary;
	axEndpoints[1].pcRemoteIp = pxOptions->pcRemoteIp;
	axEndpoints[1].uRemotePort = pxOptions->uRemotePortSecondary;
}

static void vConfigureChannelManager(
	rsrx_session_config_t * pxConfig,
	const app_options_t * pxOptions)
{
	pxConfig->eDefaultChannelId = RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.eMode =
		(pxOptions->uEnableRedundant != 0U) ?
			RSRX_REDUNDANCY_MODE_ACTIVE_STANDBY :
			RSRX_REDUNDANCY_MODE_SINGLE;
	pxConfig->xChannelManagerConfig.axChannels[0].eChannelId =
		RSRX_TRANSPORT_CHANNEL_PRIMARY;
	pxConfig->xChannelManagerConfig.axChannels[0].uPriority = 0U;
	pxConfig->xChannelManagerConfig.axChannels[0].uIsAvailable = 1U;
	pxConfig->xChannelManagerConfig.axChannels[1].eChannelId =
		RSRX_TRANSPORT_CHANNEL_SECONDARY;
	pxConfig->xChannelManagerConfig.axChannels[1].uPriority = 1U;
	pxConfig->xChannelManagerConfig.axChannels[1].uIsAvailable =
		(pxOptions->uEnableRedundant != 0U) ? 1U : 0U;
	pxConfig->xChannelManagerConfig.uPreferredChannelIndex = 0U;
	pxConfig->xChannelManagerConfig.uChannelCount =
		(pxOptions->uEnableRedundant != 0U) ? 2U : 1U;
	pxConfig->xChannelManagerConfig.uPreferredRecoveryHoldoffSelections =
		pxOptions->uPreferredRecoveryHoldoffSelections;
}

static void vConfigureCallbacks(
	rsrx_session_config_t * pxConfig,
	app_context_t * pxAppContext)
{
	pxConfig->pvApplicationDataContext = pxAppContext;
	pxConfig->pfApplicationData = on_application_data;
	pxConfig->pvApiCallbackContext = pxAppContext;
	pxConfig->pfApiNotification = on_api_notification;
	pxConfig->pvLifecycleCallbackContext = pxAppContext;
	pxConfig->pfLifecycleNotification = on_lifecycle_notification;
}

static void vBuildSessionConfig(
	rsrx_session_config_t * pxConfig,
	const app_options_t * pxOptions,
	const rsrx_transport_port_t * pxTransportPort,
	const rsrx_platform_port_table_t * pxPlatformTable,
	app_context_t * pxAppContext)
{
	static uint8_t auEncodeBuffer[1024];

	(void)memset(pxConfig, 0, sizeof(*pxConfig));
	pxConfig->xTransportPort = *pxTransportPort;
	pxConfig->xCodecPort = *rsrx_codec_get_default_port();
	pxConfig->xPlatformPorts = *pxPlatformTable;
	vConfigureChannelManager(pxConfig, pxOptions);
	pxConfig->puFramePayload = auEncodeBuffer;
	pxConfig->xFramePayloadLength = sizeof(auEncodeBuffer);
	pxConfig->uSupervisionIntervalNs = 3000000000ULL;
	pxConfig->uRetransmissionIntervalNs = 2000000000ULL;
	pxConfig->uDiagnosticFlushIntervalNs = 5000000000ULL;
	pxConfig->uBusyRejectErrorThreshold = 5U;
	pxConfig->uRequireCrc = 0U;
	pxConfig->uRequireMac = 0U;
	pxConfig->uRequireTimestamp = 0U;
	vConfigureCallbacks(pxConfig, pxAppContext);
}

static int iBuildSelectSet(
	const posix_transport_context_t * pxTransportContext,
	fd_set * pxReadFds)
{
	int iMaxFd;
	uint32_t uFdIndex;

	FD_ZERO(pxReadFds);
	iMaxFd = -1;
	for(uFdIndex = 0U;
		uFdIndex < posix_transport_get_fd_count(pxTransportContext);
		++uFdIndex)
	{
		int iSocketFd;

		iSocketFd = posix_transport_get_fd(pxTransportContext, uFdIndex);
		if(iSocketFd >= 0)
		{
			FD_SET(iSocketFd, pxReadFds);
			if(iSocketFd > iMaxFd)
			{
				iMaxFd = iSocketFd;
			}
		}
	}

	return iMaxFd;
}

static void vPollSupervisorReceive(
	rsrx_transport_supervisor_context_t * pxSupervisor,
	app_context_t * pxAppContext)
{
	const rsrx_transport_supervisor_report_t * pxSupervisorReport;
	rsrx_supervisor_status_t eSupervisorStatus;

	eSupervisorStatus = rsrx_transport_supervisor_poll_receive(
		pxSupervisor,
		&pxSupervisorReport);
	if((eSupervisorStatus != RSRX_SUPERVISOR_STATUS_OK) &&
		(eSupervisorStatus != RSRX_SUPERVISOR_STATUS_NO_FRAME) &&
		(eSupervisorStatus != RSRX_SUPERVISOR_STATUS_IGNORED_EVENT))
	{
		(void)printf(
			"Supervisor receive status=%u decision=%u effective_event=%u session_status=%u\n",
			(unsigned int)eSupervisorStatus,
			(unsigned int)pxSupervisorReport->eLastDecision,
			(unsigned int)pxSupervisorReport->eLastEffectiveEvent,
			(unsigned int)pxSupervisorReport->eLastSessionStatus);
	}
	else if((pxSupervisorReport != (const rsrx_transport_supervisor_report_t *)0) &&
		(pxSupervisorReport->eLastDecision == RSRX_SUPERVISOR_DECISION_SESSION_REJECTED))
	{
		(void)printf(
			"Supervisor rejected event=%u message_type=%u session_status=%u reason=0x%04X\n",
			(unsigned int)pxSupervisorReport->eLastEffectiveEvent,
			(unsigned int)pxSupervisorReport->xLastMessage.eMessageType,
			(unsigned int)pxSupervisorReport->eLastSessionStatus,
			(unsigned int)pxSupervisorReport->xLastMessage.eReason);
	}
	vLogSupervisorReport(pxAppContext, pxSupervisorReport);
}

static void vProcessTimerExpiries(
	rsrx_session_t * pxSession,
	posix_timer_context_t * pxTimerContext,
	const rsrx_platform_port_table_t * pxPlatformTable,
	rsrx_monotonic_time_ns_t * puNowNs,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	(void)pxPlatformTable->xClock.pfNow(pxPlatformTable->xClock.pvContext, puNowNs);

	if((pxTimerContext->uSupervisionActive != 0U) &&
		(*puNowNs >= pxTimerContext->uSupervisionDeadline))
	{
		pxTimerContext->uSupervisionActive = 0U;
		(void)rsrx_session_process_timer_expiry(
			pxSession,
			RSRX_TIMER_EXPIRY_SUPERVISION,
			ppxReport);
	}
	if((pxTimerContext->uRetransmissionActive != 0U) &&
		(*puNowNs >= pxTimerContext->uRetransmissionDeadline))
	{
		pxTimerContext->uRetransmissionActive = 0U;
		(void)rsrx_session_process_timer_expiry(
			pxSession,
			RSRX_TIMER_EXPIRY_RETRANSMISSION,
			ppxReport);
	}
	if((pxTimerContext->uDiagnosticFlushActive != 0U) &&
		(*puNowNs >= pxTimerContext->uDiagnosticFlushDeadline))
	{
		pxTimerContext->uDiagnosticFlushActive = 0U;
		(void)rsrx_session_process_timer_expiry(
			pxSession,
			RSRX_TIMER_EXPIRY_DIAGNOSTIC_FLUSH,
			ppxReport);
	}
}

static void vProcessDemoTriggers(
	const app_options_t * pxOptions,
	rsrx_monotonic_time_ns_t uStartTimeNs,
	rsrx_monotonic_time_ns_t uNowNs,
	uint32_t * puPrimaryDownInjected,
	uint32_t * puPrimaryUpInjected,
	const app_context_t * pxAppContext,
	posix_transport_context_t * pxTransportContext,
	rsrx_transport_supervisor_context_t * pxSupervisor,
	app_context_t * pxMutableAppContext)
{
	if((pxOptions->uEnableRedundant != 0U) &&
		(*puPrimaryDownInjected == 0U) &&
		(pxOptions->uPrimaryDownAfterReceiveCount > 0U) &&
		(pxAppContext->uReceiveCount >= pxOptions->uPrimaryDownAfterReceiveCount))
	{
		*puPrimaryDownInjected = 1U;
		vProcessChannelToggle(
			pxTransportContext,
			pxSupervisor,
			pxMutableAppContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			RSRX_TRANSPORT_EVENT_CHANNEL_DOWN,
			"forced down after rx threshold");
	}
	else if((pxOptions->uEnableRedundant != 0U) &&
		(*puPrimaryDownInjected == 0U) &&
		(pxOptions->uPrimaryDownAtSec > 0U) &&
		((uNowNs - uStartTimeNs) >=
			((rsrx_monotonic_time_ns_t)pxOptions->uPrimaryDownAtSec * 1000000000ULL)))
	{
		*puPrimaryDownInjected = 1U;
		vProcessChannelToggle(
			pxTransportContext,
			pxSupervisor,
			pxMutableAppContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			RSRX_TRANSPORT_EVENT_CHANNEL_DOWN,
			"forced down");
	}

	if((pxOptions->uEnableRedundant != 0U) &&
		(*puPrimaryUpInjected == 0U) &&
		(pxOptions->uPrimaryUpAtSec > 0U) &&
		((uNowNs - uStartTimeNs) >=
			((rsrx_monotonic_time_ns_t)pxOptions->uPrimaryUpAtSec * 1000000000ULL)))
	{
		*puPrimaryUpInjected = 1U;
		vProcessChannelToggle(
			pxTransportContext,
			pxSupervisor,
			pxMutableAppContext,
			RSRX_TRANSPORT_CHANNEL_PRIMARY,
			RSRX_TRANSPORT_EVENT_CHANNEL_UP,
			"restored up");
	}
}

static void vProcessClientSend(
	const app_options_t * pxOptions,
	rsrx_session_t * pxSession,
	app_context_t * pxAppContext,
	rsrx_monotonic_time_ns_t uNowNs,
	rsrx_monotonic_time_ns_t * puLastClientSendTimeNs)
{
	rsrx_status_t eSendStatus;

	if((pxOptions->uIsServer != 0U) ||
		(rsrx_session_get_state(pxSession) != RSRX_STATE_ESTABLISHED) ||
		((uNowNs - *puLastClientSendTimeNs) <
			((rsrx_monotonic_time_ns_t)pxOptions->uSendIntervalMs * 1000000ULL)))
	{
		return;
	}

	eSendStatus = rsrx_session_send_application_data(
		pxSession,
		(const uint8_t *)pxOptions->pcMessage,
		strlen(pxOptions->pcMessage));
	if(eSendStatus == RSRX_STATUS_OK)
	{
		pxAppContext->uSendCount++;
		(void)printf(
			"TX[%u]: %s\n",
			(unsigned int)pxAppContext->uSendCount,
			pxOptions->pcMessage);
	}
	else
	{
		(void)printf("application send status=%u\n", (unsigned int)eSendStatus);
	}

	*puLastClientSendTimeNs = uNowNs;
}

static void vDisconnectBeforeCleanup(
	rsrx_session_t * pxSession,
	const rsrx_orchestrator_report_t ** ppxReport)
{
	rsrx_status_t eStatus;

	if(rsrx_session_get_state(pxSession) != RSRX_STATE_ESTABLISHED)
	{
		return;
	}

	eStatus = rsrx_session_disconnect(pxSession, ppxReport);
	(void)printf("Graceful disconnect status=%u\n", (unsigned int)eStatus);
}

int main(
	int argc,
	char * argv[])
{
	app_options_t xOptions;
	posix_transport_endpoint_t axEndpoints[MAX_CHANNELS];
	posix_transport_context_t xTransportContext;
	rsrx_transport_port_t xTransportPort;
	posix_timer_context_t xTimerContext;
	rsrx_platform_port_table_t xPlatformTable;
	rsrx_session_config_t xConfig;
	rsrx_session_t xSession;
	app_context_t xAppContext;
	const rsrx_orchestrator_report_t * pxReport;
	rsrx_transport_supervisor_context_t xSupervisor;
	rsrx_monotonic_time_ns_t uStartTimeNs;
	rsrx_monotonic_time_ns_t uLastClientSendTimeNs;
	uint32_t uPrimaryDownInjected;
	uint32_t uPrimaryUpInjected;

	if(iParseArguments(argc, argv, &xOptions) != 0)
	{
		vPrintUsage(argv[0]);
		return EXIT_FAILURE;
	}

	/* Build the concrete UDP endpoints that back the abstract transport port. */
	vBuildTransportEndpoints(&xOptions, axEndpoints);

	if(posix_transport_init(
		&xTransportContext,
		&xTransportPort,
		axEndpoints,
		(xOptions.uEnableRedundant != 0U) ? 2U : 1U) != 0)
	{
		return EXIT_FAILURE;
	}

	/* Assemble the session config that application developers usually need to
	 * understand first: transport, codec, platform ports, callbacks, and
	 * channel-manager topology.
	 */
	posix_platform_init(&xPlatformTable, &xTimerContext);
	(void)memset(&xAppContext, 0, sizeof(xAppContext));
	xAppContext.pxSession = &xSession;
	xAppContext.uIsServer = xOptions.uIsServer;
	xAppContext.eLastObservedChannelId = RSRX_TRANSPORT_CHANNEL_INVALID;
	vBuildSessionConfig(
		&xConfig,
		&xOptions,
		&xTransportPort,
		&xPlatformTable,
		&xAppContext);

	if(rsrx_session_init(&xSession, &xConfig) != RSRX_STATUS_OK)
	{
		(void)fprintf(stderr, "session init failed\n");
		posix_transport_cleanup(&xTransportContext);
		return EXIT_FAILURE;
	}

	pxReport = (const rsrx_orchestrator_report_t *)0;
	if(rsrx_session_start(&xSession, &pxReport) != RSRX_STATUS_OK)
	{
		(void)fprintf(stderr, "session start failed\n");
		posix_transport_cleanup(&xTransportContext);
		return EXIT_FAILURE;
	}

	if(rsrx_transport_supervisor_init(
		&xSupervisor,
		&xSession,
		rsrx_codec_get_default_port()) != RSRX_SUPERVISOR_STATUS_OK)
	{
		(void)fprintf(stderr, "supervisor init failed\n");
		posix_transport_cleanup(&xTransportContext);
		return EXIT_FAILURE;
	}

	/* Only the client actively initiates the handshake in this example. */
	if(xOptions.uIsServer == 0U)
	{
		if(rsrx_session_connect(&xSession, &pxReport) != RSRX_STATUS_OK)
		{
			(void)fprintf(stderr, "session connect failed\n");
			posix_transport_cleanup(&xTransportContext);
			return EXIT_FAILURE;
		}
	}

	if(posix_transport_get_fd_count(&xTransportContext) == 0U)
	{
		(void)fprintf(stderr, "no transport channels available\n");
		posix_transport_cleanup(&xTransportContext);
		return EXIT_FAILURE;
	}

	(void)signal(SIGINT, vHandleSignal);
	(void)signal(SIGTERM, vHandleSignal);
	(void)xPlatformTable.xClock.pfNow(
		xPlatformTable.xClock.pvContext,
		&uStartTimeNs);
	uLastClientSendTimeNs = uStartTimeNs;
	uPrimaryDownInjected = 0U;
	uPrimaryUpInjected = 0U;

	(void)printf(
		"Starting %s local=%u remote=%s:%u redundant=%u secondary_local=%u secondary_remote=%u holdoff=%u interval_ms=%u duration_sec=%u down_at_sec=%u down_after_rx=%u up_at_sec=%u\n",
		xOptions.pcRole,
		(unsigned int)xOptions.uLocalPort,
		xOptions.pcRemoteIp,
		(unsigned int)xOptions.uRemotePort,
		(unsigned int)xOptions.uEnableRedundant,
		(unsigned int)xOptions.uLocalPortSecondary,
		(unsigned int)xOptions.uRemotePortSecondary,
		(unsigned int)xOptions.uPreferredRecoveryHoldoffSelections,
		(unsigned int)xOptions.uSendIntervalMs,
		(unsigned int)xOptions.uRunDurationSec,
		(unsigned int)xOptions.uPrimaryDownAtSec,
		(unsigned int)xOptions.uPrimaryDownAfterReceiveCount,
		(unsigned int)xOptions.uPrimaryUpAtSec);

	while(g_iKeepRunning != 0)
	{
		fd_set xReadFds;
		struct timeval xTimeout;
		int iSelectResult;
		int iMaxFd;
		rsrx_monotonic_time_ns_t uNowNs;

		/* Poll every configured UDP channel and let the transport supervisor
		 * convert frames into session events.
		 */
		iMaxFd = iBuildSelectSet(&xTransportContext, &xReadFds);
		xTimeout.tv_sec = 0;
		xTimeout.tv_usec = 10000;
		iSelectResult = select(iMaxFd + 1, &xReadFds, (fd_set *)0, (fd_set *)0, &xTimeout);
		if(iSelectResult > 0)
		{
			vPollSupervisorReceive(&xSupervisor, &xAppContext);
		}

		/* Drive the three timers that the session relies on. */
		vProcessTimerExpiries(
			&xSession,
			&xTimerContext,
			&xPlatformTable,
			&uNowNs,
			&pxReport);
		vProcessDemoTriggers(
			&xOptions,
			uStartTimeNs,
			uNowNs,
			&uPrimaryDownInjected,
			&uPrimaryUpInjected,
			&xAppContext,
			&xTransportContext,
			&xSupervisor,
			&xAppContext);

		/* After ESTABLISHED, the client periodically emits application data. */
		vProcessClientSend(
			&xOptions,
			&xSession,
			&xAppContext,
			uNowNs,
			&uLastClientSendTimeNs);

		if((xOptions.uRunDurationSec > 0U) &&
			((uNowNs - uStartTimeNs) >=
				((rsrx_monotonic_time_ns_t)xOptions.uRunDurationSec * 1000000000ULL)))
		{
			vLogWithTimestamp("", "Run duration reached, stopping.");
			break;
		}

		if(rsrx_session_get_state(&xSession) == RSRX_STATE_SAFE_DISCONNECT)
		{
			vLogWithTimestamp("", "Session entered SAFE_DISCONNECT, stopping.");
			break;
		}
	}

	vDisconnectBeforeCleanup(&xSession, &pxReport);
	posix_transport_cleanup(&xTransportContext);
	(void)printf(
		"Stopped role=%s tx=%u rx=%u final_state=%u\n",
		xOptions.pcRole,
		(unsigned int)xAppContext.uSendCount,
		(unsigned int)xAppContext.uReceiveCount,
		(unsigned int)rsrx_session_get_state(&xSession));
	return EXIT_SUCCESS;
}
