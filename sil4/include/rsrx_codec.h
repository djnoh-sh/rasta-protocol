#ifndef RSRX_CODEC_H
#define RSRX_CODEC_H

#include <stddef.h>
#include <stdint.h>

#include "rsrx_state_machine.h"
#include "rsrx_transport.h"

#define D_RSRX_CODEC_HEADER_BYTES (16U)
#define D_RSRX_CODEC_MAX_PAYLOAD_BYTES (512U)
#define D_RSRX_CODEC_MAX_FRAME_BYTES (D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_MAX_PAYLOAD_BYTES)
#define D_RSRX_CODEC_CRC_BYTES (4U)
#define D_RSRX_CODEC_MAX_CRC_FRAME_BYTES (D_RSRX_CODEC_MAX_FRAME_BYTES + D_RSRX_CODEC_CRC_BYTES)

typedef enum
{
	RSRX_CODEC_STATUS_OK = 0,
	RSRX_CODEC_STATUS_INVALID_ARGUMENT,
	RSRX_CODEC_STATUS_BUFFER_TOO_SMALL,
	RSRX_CODEC_STATUS_DECODE_ERROR,
	RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
	RSRX_CODEC_STATUS_CRC_MISMATCH
} rsrx_codec_status_t;

typedef enum
{
	RSRX_MESSAGE_TYPE_INVALID = 0,
	RSRX_MESSAGE_TYPE_CONNECT_REQUEST,
	RSRX_MESSAGE_TYPE_CONNECT_RESPONSE,
	RSRX_MESSAGE_TYPE_HEARTBEAT,
	RSRX_MESSAGE_TYPE_DATA,
	RSRX_MESSAGE_TYPE_RETRANSMISSION_REQUEST,
	RSRX_MESSAGE_TYPE_DISCONNECT,
	RSRX_MESSAGE_TYPE_DIAGNOSTIC
} rsrx_message_type_t;

typedef struct
{
	rsrx_message_type_t eMessageType;
	rsrx_event_t eSuggestedEvent;
	rsrx_reason_code_t eReason;
	uint32_t uSequenceNumber;
	uint32_t uConfirmationNumber;
	uint8_t auPayload[D_RSRX_CODEC_MAX_PAYLOAD_BYTES];
	size_t xPayloadLength;
} rsrx_decoded_message_t;

typedef struct
{
	rsrx_message_type_t eMessageType;
	rsrx_reason_code_t eReason;
	uint32_t uSequenceNumber;
	uint32_t uConfirmationNumber;
	const uint8_t * puPayload;
	size_t xPayloadLength;
} rsrx_encode_request_t;

typedef struct
{
	size_t xHeaderBytes;
	size_t xMaxPayloadBytes;
	size_t xMaxFrameBytes;
	uint32_t uCrcPresent;
	uint32_t uMacPresent;
	uint32_t uTimestampPresent;
} rsrx_codec_wire_profile_t;

typedef struct
{
	uint8_t * puBuffer;
	size_t xBufferCapacity;
	size_t xEncodedLength;
} rsrx_encode_buffer_t;

typedef rsrx_codec_status_t (*rsrx_encode_message_fn)(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

typedef rsrx_codec_status_t (*rsrx_decode_frame_fn)(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage);

typedef struct
{
	rsrx_encode_message_fn pfEncode;
	rsrx_decode_frame_fn pfDecode;
} rsrx_codec_port_t;

rsrx_codec_status_t rsrx_codec_encode_message(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

rsrx_codec_status_t rsrx_codec_decode_frame(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage);

const rsrx_codec_port_t * rsrx_codec_get_default_port(void);

const rsrx_codec_port_t * rsrx_codec_get_crc32_port(void);

const rsrx_codec_wire_profile_t * rsrx_codec_get_wire_profile(void);

const rsrx_codec_wire_profile_t * rsrx_codec_get_crc32_wire_profile(void);

rsrx_codec_status_t rsrx_codec_calculate_crc32(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc);

rsrx_codec_status_t rsrx_codec_encode_message_with_crc32(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

rsrx_codec_status_t rsrx_codec_decode_frame_with_crc32(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage);

#endif
