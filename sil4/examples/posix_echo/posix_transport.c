#include "posix_transport.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static posix_udp_channel_t * pxGetChannel(
	posix_transport_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId)
{
	uint32_t uIndex;

	for(uIndex = 0U; uIndex < pxContext->uChannelCount; ++uIndex)
	{
		if(pxContext->axChannels[uIndex].eChannelId == eChannelId)
		{
			return &pxContext->axChannels[uIndex];
		}
	}

	return (posix_udp_channel_t *)0;
}

static uint32_t uRemoteMatchesExpected(
	const posix_udp_channel_t * pxChannel,
	const struct sockaddr_in * pxRemoteAddress)
{
	return (uint32_t)((pxRemoteAddress->sin_family == AF_INET) &&
		(pxRemoteAddress->sin_port == pxChannel->xRemoteAddress.sin_port) &&
		(pxRemoteAddress->sin_addr.s_addr == pxChannel->xRemoteAddress.sin_addr.s_addr));
}

static rsrx_transport_status_t posix_transport_send(
	void * pvContext,
	const rsrx_transport_send_request_t * pxRequest)
{
	posix_transport_context_t * pxContext = (posix_transport_context_t *)pvContext;
	posix_udp_channel_t * pxChannel;
	ssize_t iBytesSent;

	if((pxContext == (posix_transport_context_t *)0) ||
		(pxRequest == (const rsrx_transport_send_request_t *)0) ||
		(pxRequest->puPayload == (const uint8_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	pxChannel = pxGetChannel(pxContext, pxRequest->eChannelId);
	if(pxChannel == (posix_udp_channel_t *)0)
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	if((pxChannel->iIsUp == 0) || (pxChannel->iSocketFd < 0))
	{
		return RSRX_TRANSPORT_STATUS_CHANNEL_DOWN;
	}

	iBytesSent = sendto(
		pxChannel->iSocketFd,
		pxRequest->puPayload,
		pxRequest->xPayloadLength,
		0,
		(const struct sockaddr *)&pxChannel->xRemoteAddress,
		sizeof(pxChannel->xRemoteAddress));

	if(iBytesSent < 0)
	{
		if((errno == EAGAIN) || (errno == EWOULDBLOCK))
		{
			return RSRX_TRANSPORT_STATUS_UNAVAILABLE;
		}

		return RSRX_TRANSPORT_STATUS_TX_ERROR;
	}

	return RSRX_TRANSPORT_STATUS_OK;
}

static rsrx_transport_status_t posix_transport_receive(
	void * pvContext,
	rsrx_transport_frame_t * pxFrame)
{
	posix_transport_context_t * pxContext = (posix_transport_context_t *)pvContext;
	posix_udp_channel_t * pxChannel;
	struct sockaddr_in xSourceAddress;
	socklen_t xSourceAddressLength;
	ssize_t iBytesReceived;
	uint32_t uIndex;
	uint32_t uFoundRequestedChannel;
	rsrx_transport_channel_id_t eRequestedChannelId;

	if((pxContext == (posix_transport_context_t *)0) ||
		(pxFrame == (rsrx_transport_frame_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	eRequestedChannelId = pxFrame->eChannelId;
	uFoundRequestedChannel = 0U;
	for(uIndex = 0U; uIndex < pxContext->uChannelCount; ++uIndex)
	{
		pxChannel = &pxContext->axChannels[uIndex];
		if((eRequestedChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
			(pxChannel->eChannelId != eRequestedChannelId))
		{
			continue;
		}
		uFoundRequestedChannel = 1U;

		if((pxChannel->iIsUp == 0) || (pxChannel->iSocketFd < 0))
		{
			continue;
		}

		xSourceAddressLength = sizeof(xSourceAddress);
		iBytesReceived = recvfrom(
			pxChannel->iSocketFd,
			pxContext->auReceiveBuffer,
			sizeof(pxContext->auReceiveBuffer),
			0,
			(struct sockaddr *)&xSourceAddress,
			&xSourceAddressLength);

		if(iBytesReceived < 0)
		{
			if((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				continue;
			}

			return RSRX_TRANSPORT_STATUS_RX_ERROR;
		}

		if(uRemoteMatchesExpected(pxChannel, &xSourceAddress) == 0U)
		{
			return RSRX_TRANSPORT_STATUS_RX_ERROR;
		}

		pxFrame->eChannelId = pxChannel->eChannelId;
		pxFrame->puPayload = pxContext->auReceiveBuffer;
		pxFrame->xPayloadLength = (size_t)iBytesReceived;
		pxFrame->eEventType = RSRX_TRANSPORT_EVENT_FRAME_RECEIVED;
		return RSRX_TRANSPORT_STATUS_OK;
	}

	if((eRequestedChannelId != RSRX_TRANSPORT_CHANNEL_INVALID) &&
		(uFoundRequestedChannel == 0U))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	return RSRX_TRANSPORT_STATUS_UNAVAILABLE;
}

static rsrx_transport_status_t posix_transport_channel_query(
	void * pvContext,
	rsrx_transport_channel_state_t * pxState)
{
	posix_transport_context_t * pxContext = (posix_transport_context_t *)pvContext;
	posix_udp_channel_t * pxChannel;

	if((pxContext == (posix_transport_context_t *)0) ||
		(pxState == (rsrx_transport_channel_state_t *)0))
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	pxChannel = pxGetChannel(pxContext, pxState->eChannelId);
	if(pxChannel == (posix_udp_channel_t *)0)
	{
		return RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT;
	}

	pxState->uIsAvailable = (uint32_t)(pxChannel->iIsUp != 0);
	return RSRX_TRANSPORT_STATUS_OK;
}

static int iInitSocketChannel(
	posix_udp_channel_t * pxChannel,
	const posix_transport_endpoint_t * pxEndpoint)
{
	struct sockaddr_in xLocalAddress;
	int iFlags;
	int iReuseAddress;

	pxChannel->eChannelId = pxEndpoint->eChannelId;
	pxChannel->iSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if(pxChannel->iSocketFd < 0)
	{
		perror("socket");
		return -1;
	}

	iReuseAddress = 1;
	if(setsockopt(
		pxChannel->iSocketFd,
		SOL_SOCKET,
		SO_REUSEADDR,
		&iReuseAddress,
		sizeof(iReuseAddress)) != 0)
	{
		perror("setsockopt");
		close(pxChannel->iSocketFd);
		pxChannel->iSocketFd = -1;
		return -1;
	}

	iFlags = fcntl(pxChannel->iSocketFd, F_GETFL, 0);
	if(iFlags < 0)
	{
		perror("fcntl(F_GETFL)");
		close(pxChannel->iSocketFd);
		pxChannel->iSocketFd = -1;
		return -1;
	}

	if(fcntl(pxChannel->iSocketFd, F_SETFL, iFlags | O_NONBLOCK) != 0)
	{
		perror("fcntl(F_SETFL)");
		close(pxChannel->iSocketFd);
		pxChannel->iSocketFd = -1;
		return -1;
	}

	(void)memset(&xLocalAddress, 0, sizeof(xLocalAddress));
	xLocalAddress.sin_family = AF_INET;
	xLocalAddress.sin_addr.s_addr = INADDR_ANY;
	xLocalAddress.sin_port = htons(pxEndpoint->uLocalPort);

	if(bind(
		pxChannel->iSocketFd,
		(struct sockaddr *)&xLocalAddress,
		sizeof(xLocalAddress)) != 0)
	{
		perror("bind");
		close(pxChannel->iSocketFd);
		pxChannel->iSocketFd = -1;
		return -1;
	}

	(void)memset(&pxChannel->xRemoteAddress, 0, sizeof(pxChannel->xRemoteAddress));
	pxChannel->xRemoteAddress.sin_family = AF_INET;
	pxChannel->xRemoteAddress.sin_port = htons(pxEndpoint->uRemotePort);
	if(inet_pton(AF_INET, pxEndpoint->pcRemoteIp, &pxChannel->xRemoteAddress.sin_addr) != 1)
	{
		(void)fprintf(stderr, "invalid remote ip: %s\n", pxEndpoint->pcRemoteIp);
		close(pxChannel->iSocketFd);
		pxChannel->iSocketFd = -1;
		return -1;
	}

	pxChannel->iIsUp = 1;
	return 0;
}

int posix_transport_init(
	posix_transport_context_t * pxContext,
	rsrx_transport_port_t * pxPort,
	const posix_transport_endpoint_t * pxEndpoints,
	uint32_t uEndpointCount)
{
	uint32_t uIndex;

	if((pxContext == (posix_transport_context_t *)0) ||
		(pxPort == (rsrx_transport_port_t *)0) ||
		(pxEndpoints == (const posix_transport_endpoint_t *)0) ||
		(uEndpointCount == 0U) ||
		(uEndpointCount > MAX_CHANNELS))
	{
		return -1;
	}

	(void)memset(pxContext, 0, sizeof(*pxContext));
	for(uIndex = 0U; uIndex < MAX_CHANNELS; ++uIndex)
	{
		pxContext->axChannels[uIndex].iSocketFd = -1;
	}

	for(uIndex = 0U; uIndex < uEndpointCount; ++uIndex)
	{
		if((pxEndpoints[uIndex].pcRemoteIp == (const char *)0) ||
			(pxEndpoints[uIndex].eChannelId == RSRX_TRANSPORT_CHANNEL_INVALID))
		{
			posix_transport_cleanup(pxContext);
			return -1;
		}

		if(iInitSocketChannel(&pxContext->axChannels[uIndex], &pxEndpoints[uIndex]) != 0)
		{
			posix_transport_cleanup(pxContext);
			return -1;
		}
	}

	pxContext->uChannelCount = uEndpointCount;

	pxPort->pvContext = pxContext;
	pxPort->pfSend = posix_transport_send;
	pxPort->pfReceive = posix_transport_receive;
	pxPort->pfQueryChannel = posix_transport_channel_query;

	return 0;
}

uint32_t posix_transport_get_fd_count(
	const posix_transport_context_t * pxContext)
{
	if(pxContext == (const posix_transport_context_t *)0)
	{
		return 0U;
	}

	return pxContext->uChannelCount;
}

int posix_transport_get_fd(
	const posix_transport_context_t * pxContext,
	uint32_t uIndex)
{
	if((pxContext == (const posix_transport_context_t *)0) ||
		(uIndex >= pxContext->uChannelCount))
	{
		return -1;
	}

	return pxContext->axChannels[uIndex].iSocketFd;
}

int posix_transport_set_channel_availability(
	posix_transport_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId,
	uint32_t uIsUp)
{
	posix_udp_channel_t * pxChannel;

	if(pxContext == (posix_transport_context_t *)0)
	{
		return -1;
	}

	pxChannel = pxGetChannel(pxContext, eChannelId);
	if(pxChannel == (posix_udp_channel_t *)0)
	{
		return -1;
	}

	pxChannel->iIsUp = (int)(uIsUp != 0U);
	return 0;
}

void posix_transport_cleanup(
	posix_transport_context_t * pxContext)
{
	uint32_t uIndex;

	if(pxContext == (posix_transport_context_t *)0)
	{
		return;
	}

	for(uIndex = 0U; uIndex < pxContext->uChannelCount; ++uIndex)
	{
		if(pxContext->axChannels[uIndex].iSocketFd >= 0)
		{
			close(pxContext->axChannels[uIndex].iSocketFd);
			pxContext->axChannels[uIndex].iSocketFd = -1;
		}
	}
}
