#ifndef POSIX_TRANSPORT_H
#define POSIX_TRANSPORT_H

#include "rsrx_transport.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#define MAX_CHANNELS 2
#define MAX_RECV_BUFFER 1024

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	uint16_t uLocalPort;
	const char * pcRemoteIp;
	uint16_t uRemotePort;
} posix_transport_endpoint_t;

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	int iSocketFd;
	int iIsUp;
	struct sockaddr_in xRemoteAddress;
} posix_udp_channel_t;

typedef struct
{
	posix_udp_channel_t axChannels[MAX_CHANNELS];
	uint32_t uChannelCount;
	uint8_t auReceiveBuffer[MAX_RECV_BUFFER];
} posix_transport_context_t;

int posix_transport_init(
	posix_transport_context_t * pxContext,
	rsrx_transport_port_t * pxPort,
	const posix_transport_endpoint_t * pxEndpoints,
	uint32_t uEndpointCount);

uint32_t posix_transport_get_fd_count(
	const posix_transport_context_t * pxContext);

int posix_transport_get_fd(
	const posix_transport_context_t * pxContext,
	uint32_t uIndex);

int posix_transport_set_channel_availability(
	posix_transport_context_t * pxContext,
	rsrx_transport_channel_id_t eChannelId,
	uint32_t uIsUp);

void posix_transport_cleanup(
	posix_transport_context_t * pxContext);

#endif
