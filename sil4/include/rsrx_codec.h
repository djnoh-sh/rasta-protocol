#ifndef RSRX_CODEC_H
#define RSRX_CODEC_H

#include <stddef.h>
#include <stdint.h>

#include "rsrx_state_machine.h"
#include "rsrx_transport.h"

#define D_RSRX_CODEC_HEADER_BYTES (16U)
#define D_RSRX_CODEC_RASTA_SR_HEADER_BYTES (28U)
#define D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES (8U)
#define D_RSRX_CODEC_MAX_PAYLOAD_BYTES (512U)
#define D_RSRX_CODEC_MAX_FRAME_BYTES (D_RSRX_CODEC_HEADER_BYTES + D_RSRX_CODEC_MAX_PAYLOAD_BYTES)
#define D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES (D_RSRX_CODEC_RASTA_SR_HEADER_BYTES + D_RSRX_CODEC_MAX_PAYLOAD_BYTES)
#define D_RSRX_CODEC_CRC_BYTES (4U)
#define D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES (4U)
#define D_RSRX_CODEC_MAX_RASTA_REDUNDANCY_FRAME_BYTES \
	(D_RSRX_CODEC_RASTA_REDUNDANCY_HEADER_BYTES + \
		D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES + \
		D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES)
#define D_RSRX_CODEC_RASTA_SR_TIMESTAMP_BYTES (8U)
#define D_RSRX_CODEC_RASTA_SR_MAX_CHECKSUM_BYTES (16U)
#define D_RSRX_CODEC_MAX_CRC_FRAME_BYTES (D_RSRX_CODEC_MAX_FRAME_BYTES + D_RSRX_CODEC_CRC_BYTES)
#define D_RSRX_CODEC_WIRE_PROFILE_DEFAULT (1U)
#define D_RSRX_CODEC_WIRE_PROFILE_CRC32 (2U)
#define D_RSRX_CODEC_WIRE_PROFILE_RASTA_SR (3U)
#define D_RSRX_CODEC_WIRE_PROFILE_RASTA_REDUNDANCY (4U)
#define D_RSRX_CODEC_WIRE_PROFILE_VERSION (1U)
#define D_RSRX_CODEC_RASTA_SR_BYTE_ORDER_BIG_ENDIAN (1U)

typedef enum
{
	RSRX_CODEC_STATUS_OK = 0,
	RSRX_CODEC_STATUS_INVALID_ARGUMENT,
	RSRX_CODEC_STATUS_BUFFER_TOO_SMALL,
	RSRX_CODEC_STATUS_DECODE_ERROR,
	RSRX_CODEC_STATUS_UNSUPPORTED_MESSAGE,
	RSRX_CODEC_STATUS_CRC_MISMATCH,
	RSRX_CODEC_STATUS_CRC_TRUNCATED,
	RSRX_CODEC_STATUS_RESERVED_HEADER_NONZERO,
	RSRX_CODEC_STATUS_LENGTH_MISMATCH,
	RSRX_CODEC_STATUS_PAYLOAD_TOO_LARGE,
	RSRX_CODEC_STATUS_SHORT_HEADER,
	RSRX_CODEC_STATUS_UNSUPPORTED_REASON,
	RSRX_CODEC_STATUS_NON_FRAME_EVENT,
	RSRX_CODEC_STATUS_INVALID_CHANNEL,
	RSRX_CODEC_STATUS_TRAILING_BYTES,
	RSRX_CODEC_STATUS_TRUNCATED_PAYLOAD,
	RSRX_CODEC_STATUS_UNSUPPORTED_CHECKSUM_PROFILE,
	RSRX_CODEC_STATUS_TIMESTAMP_ZERO,
	RSRX_CODEC_STATUS_TIMESTAMP_IN_FUTURE,
	RSRX_CODEC_STATUS_TIMESTAMP_STALE,
	RSRX_CODEC_STATUS_TIMESTAMP_REGRESSED,
	RSRX_CODEC_STATUS_RECEIVER_ID_MISMATCH,
	RSRX_CODEC_STATUS_SENDER_ID_MISMATCH
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

typedef enum
{
	RSRX_RASTA_SR_TYPE_CONNREQ = 6200U,
	RSRX_RASTA_SR_TYPE_CONNRESP = 6201U,
	RSRX_RASTA_SR_TYPE_RETRREQ = 6212U,
	RSRX_RASTA_SR_TYPE_RETRRESP = 6213U,
	RSRX_RASTA_SR_TYPE_DISCREQ = 6216U,
	RSRX_RASTA_SR_TYPE_HB = 6220U,
	RSRX_RASTA_SR_TYPE_DATA = 6240U,
	RSRX_RASTA_SR_TYPE_RETRDATA = 6241U
} rsrx_rasta_sr_message_type_t;

typedef enum
{
	RSRX_RASTA_DISC_REASON_USERREQUEST = 0U,
	RSRX_RASTA_DISC_REASON_UNEXPECTEDTYPE = 2U,
	RSRX_RASTA_DISC_REASON_SEQNERROR = 3U,
	RSRX_RASTA_DISC_REASON_TIMEOUT = 4U,
	RSRX_RASTA_DISC_REASON_SERVICENOTALLOWED = 5U,
	RSRX_RASTA_DISC_REASON_INCOMPATIBLEVERSION = 6U,
	RSRX_RASTA_DISC_REASON_RETRFAILED = 7U,
	RSRX_RASTA_DISC_REASON_PROTOCOLERROR = 8U
} rsrx_rasta_disconnect_reason_t;

typedef enum
{
	RSRX_RASTA_SR_CHECKSUM_ALGORITHM_NONE = 0,
	RSRX_RASTA_SR_CHECKSUM_ALGORITHM_MD4,
	RSRX_RASTA_SR_CHECKSUM_ALGORITHM_BLAKE2B,
	RSRX_RASTA_SR_CHECKSUM_ALGORITHM_SIPHASH_2_4
} rsrx_rasta_sr_checksum_algorithm_t;

typedef enum
{
	RSRX_RASTA_REDUNDANCY_CRC_OPTION_A = 0,
	RSRX_RASTA_REDUNDANCY_CRC_OPTION_B,
	RSRX_RASTA_REDUNDANCY_CRC_OPTION_C,
	RSRX_RASTA_REDUNDANCY_CRC_OPTION_D,
	RSRX_RASTA_REDUNDANCY_CRC_OPTION_E
} rsrx_rasta_redundancy_crc_option_t;

typedef struct
{
	rsrx_rasta_sr_checksum_algorithm_t eAlgorithm;
	size_t xChecksumBytes;
} rsrx_rasta_sr_checksum_profile_t;

typedef struct
{
	rsrx_rasta_redundancy_crc_option_t eOption;
	size_t xCrcBytes;
} rsrx_rasta_redundancy_crc_profile_t;

typedef struct
{
	uint32_t uCurrentTimestamp;
	uint32_t uAcceptedPastWindow;
	uint32_t uAcceptedFutureWindow;
	uint32_t uLastAcceptedTimestamp;
} rsrx_rasta_sr_timestamp_admission_policy_t;

typedef struct
{
	uint32_t uExpectedReceiverId;
	uint32_t uExpectedSenderId;
} rsrx_rasta_sr_identity_admission_policy_t;

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
	uint16_t usPacketLength;
	uint16_t usMessageType;
	uint32_t uReceiverId;
	uint32_t uSenderId;
	uint32_t uSequenceNumber;
	uint32_t uConfirmedSequenceNumber;
	uint32_t uTimestamp;
	uint32_t uConfirmedTimestamp;
	const uint8_t * puPayload;
	size_t xPayloadLength;
	const uint8_t * puChecksum;
	size_t xChecksumLength;
} rsrx_rasta_sr_encode_request_t;

typedef struct
{
	uint16_t usPacketLength;
	uint16_t usMessageType;
	uint32_t uReceiverId;
	uint32_t uSenderId;
	uint32_t uSequenceNumber;
	uint32_t uConfirmedSequenceNumber;
	uint32_t uTimestamp;
	uint32_t uConfirmedTimestamp;
	uint8_t auPayload[D_RSRX_CODEC_MAX_PAYLOAD_BYTES];
	size_t xPayloadLength;
	uint8_t auChecksum[D_RSRX_CODEC_RASTA_SR_MAX_CHECKSUM_BYTES];
	size_t xChecksumLength;
	uint32_t uChecksumPresent;
} rsrx_rasta_sr_decoded_packet_t;

typedef struct
{
	uint16_t usPacketLength;
	uint16_t usReserve;
	uint32_t uSequenceNumber;
	const uint8_t * puCarriedPacket;
	size_t xCarriedPacketLength;
	const rsrx_rasta_redundancy_crc_profile_t * pxCrcProfile;
} rsrx_rasta_redundancy_encode_request_t;

typedef struct
{
	uint16_t usPacketLength;
	uint16_t usReserve;
	uint32_t uSequenceNumber;
	uint8_t auCarriedPacket[D_RSRX_CODEC_MAX_RASTA_SR_FRAME_BYTES];
	size_t xCarriedPacketLength;
	uint8_t auCrc[D_RSRX_CODEC_RASTA_REDUNDANCY_MAX_CRC_BYTES];
	size_t xCrcLength;
	uint32_t uCrcPresent;
} rsrx_rasta_redundancy_decoded_packet_t;

typedef struct
{
	uint32_t uProfileId;
	uint32_t uProfileVersion;
	size_t xHeaderBytes;
	size_t xMaxPayloadBytes;
	size_t xMaxFrameBytes;
	size_t xCrcBytes;
	size_t xMacBytes;
	size_t xTimestampBytes;
	uint32_t uCrcPresent;
	uint32_t uMacPresent;
	uint32_t uTimestampPresent;
} rsrx_codec_wire_profile_t;

typedef struct
{
	uint32_t uDefaultCrcPresent;
	uint32_t uOptionalCrc32Available;
	uint32_t uMacAvailable;
	uint32_t uTimestampAvailable;
} rsrx_codec_security_capabilities_t;

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

typedef rsrx_codec_status_t (*rsrx_crc32_calculate_fn)(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc);

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

const rsrx_codec_wire_profile_t * rsrx_codec_get_rasta_sr_wire_profile(void);

const rsrx_codec_wire_profile_t * rsrx_codec_get_rasta_redundancy_wire_profile(void);

const rsrx_codec_security_capabilities_t * rsrx_codec_get_security_capabilities(void);

rsrx_codec_status_t rsrx_codec_encode_rasta_sr_no_checksum(
	const rsrx_rasta_sr_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

rsrx_codec_status_t rsrx_codec_decode_rasta_sr_no_checksum(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_rasta_sr_decoded_packet_t * pxPacket);

rsrx_codec_status_t rsrx_codec_encode_rasta_redundancy_no_crc(
	const rsrx_rasta_redundancy_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

rsrx_codec_status_t rsrx_codec_decode_rasta_redundancy_no_crc(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_rasta_redundancy_decoded_packet_t * pxPacket);

rsrx_codec_status_t rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_rasta_sr_decoded_packet_t * pxPacket);

rsrx_codec_status_t rsrx_codec_validate_rasta_sr_checksum_profile(
	const rsrx_rasta_sr_checksum_profile_t * pxProfile);

const rsrx_rasta_sr_checksum_profile_t * rsrx_codec_get_rasta_sr_default_checksum_profile(void);

rsrx_codec_status_t rsrx_codec_validate_rasta_redundancy_crc_profile(
	const rsrx_rasta_redundancy_crc_profile_t * pxProfile);

rsrx_codec_status_t rsrx_codec_validate_rasta_sr_timestamp_admission(
	const rsrx_rasta_sr_decoded_packet_t * pxPacket,
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxPolicy);

rsrx_codec_status_t rsrx_codec_validate_rasta_sr_identity_admission(
	const rsrx_rasta_sr_decoded_packet_t * pxPacket,
	const rsrx_rasta_sr_identity_admission_policy_t * pxPolicy);

rsrx_codec_status_t rsrx_codec_map_rasta_sr_packet_to_message_with_timestamp_admission(
	const rsrx_rasta_sr_decoded_packet_t * pxPacket,
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxPolicy,
	rsrx_decoded_message_t * pxMessage);

rsrx_codec_status_t rsrx_codec_map_rasta_sr_packet_to_message_with_identity_and_timestamp_admission(
	const rsrx_rasta_sr_decoded_packet_t * pxPacket,
	const rsrx_rasta_sr_timestamp_admission_policy_t * pxTimestampPolicy,
	const rsrx_rasta_sr_identity_admission_policy_t * pxIdentityPolicy,
	rsrx_decoded_message_t * pxMessage);

rsrx_codec_status_t rsrx_codec_map_message_type_to_rasta_sr_type(
	rsrx_message_type_t eMessageType,
	uint16_t * pusRastaType);

rsrx_codec_status_t rsrx_codec_map_rasta_sr_type_to_message_type(
	uint16_t usRastaType,
	rsrx_message_type_t * peMessageType);

rsrx_codec_status_t rsrx_codec_map_reason_to_rasta_disconnect_reason(
	rsrx_reason_code_t eReason,
	uint16_t * pusRastaReason);

rsrx_codec_status_t rsrx_codec_write_rasta_sr_uint16(
	uint16_t usValue,
	uint8_t * puBuffer);

rsrx_codec_status_t rsrx_codec_read_rasta_sr_uint16(
	const uint8_t * puBuffer,
	uint16_t * pusValue);

rsrx_codec_status_t rsrx_codec_write_rasta_sr_uint32(
	uint32_t uValue,
	uint8_t * puBuffer);

rsrx_codec_status_t rsrx_codec_read_rasta_sr_uint32(
	const uint8_t * puBuffer,
	uint32_t * puValue);

rsrx_codec_status_t rsrx_codec_calculate_crc32(
	const uint8_t * puData,
	size_t xDataLength,
	uint32_t * puCrc);

rsrx_codec_status_t rsrx_codec_encode_message_with_crc32(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer);

rsrx_codec_status_t rsrx_codec_encode_message_with_crc32_calculator(
	const rsrx_encode_request_t * pxRequest,
	rsrx_encode_buffer_t * pxBuffer,
	rsrx_crc32_calculate_fn pfCalculateCrc32);

rsrx_codec_status_t rsrx_codec_decode_frame_with_crc32(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage);

rsrx_codec_status_t rsrx_codec_decode_frame_with_crc32_calculator(
	const rsrx_transport_frame_t * pxFrame,
	rsrx_decoded_message_t * pxMessage,
	rsrx_crc32_calculate_fn pfCalculateCrc32);

#endif
