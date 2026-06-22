#ifndef RSRX_TRANSPORT_H
#define RSRX_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

#include "rsrx_state_machine.h"

typedef enum
{
	RSRX_TRANSPORT_STATUS_OK = 0,
	RSRX_TRANSPORT_STATUS_INVALID_ARGUMENT,
	RSRX_TRANSPORT_STATUS_UNAVAILABLE,
	RSRX_TRANSPORT_STATUS_TX_ERROR,
	RSRX_TRANSPORT_STATUS_RX_ERROR,
	RSRX_TRANSPORT_STATUS_CHANNEL_DOWN
} rsrx_transport_status_t;

typedef enum
{
	RSRX_TRANSPORT_CHANNEL_INVALID = 0,
	RSRX_TRANSPORT_CHANNEL_PRIMARY,
	RSRX_TRANSPORT_CHANNEL_SECONDARY,
	RSRX_TRANSPORT_CHANNEL_REDUNDANT
} rsrx_transport_channel_id_t;

typedef enum
{
	RSRX_TRANSPORT_EVENT_NONE = 0,
	RSRX_TRANSPORT_EVENT_CHANNEL_UP,
	RSRX_TRANSPORT_EVENT_CHANNEL_DOWN,
	RSRX_TRANSPORT_EVENT_FRAME_RECEIVED,
	RSRX_TRANSPORT_EVENT_SEND_COMPLETED,
	RSRX_TRANSPORT_EVENT_SEND_FAILED
} rsrx_transport_event_type_t;

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	const uint8_t * puPayload;
	size_t xPayloadLength;
	rsrx_reason_code_t eReason;
} rsrx_transport_send_request_t;

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	const uint8_t * puPayload;
	size_t xPayloadLength;
	rsrx_transport_event_type_t eEventType;
} rsrx_transport_frame_t;

typedef struct
{
	rsrx_transport_channel_id_t eChannelId;
	uint32_t uIsAvailable;
} rsrx_transport_channel_state_t;

typedef rsrx_transport_status_t (*rsrx_transport_send_fn)(
	void * pvContext,
	const rsrx_transport_send_request_t * pxRequest);

typedef rsrx_transport_status_t (*rsrx_transport_receive_fn)(
	void * pvContext,
	rsrx_transport_frame_t * pxFrame);

typedef rsrx_transport_status_t (*rsrx_transport_channel_query_fn)(
	void * pvContext,
	rsrx_transport_channel_state_t * pxState);

typedef struct
{
	void * pvContext;
	rsrx_transport_send_fn pfSend;
	rsrx_transport_receive_fn pfReceive;
	rsrx_transport_channel_query_fn pfQueryChannel;
} rsrx_transport_port_t;

#endif
