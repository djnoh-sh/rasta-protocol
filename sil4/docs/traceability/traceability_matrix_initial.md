# Initial Traceability Matrix

| Req ID | Safety ID | Design ID | Source File | Function | Test ID | Review Record | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FR-001 |  | HLD-001 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init` |  |  | Draft |
| FR-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state` | TC-OR-001 |  | Draft |
| FR-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_init`, `rsrx_session_start` | TC-API-001 |  | Draft |
| FR-002 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-001, TC-SM-002, TC-SM-009, TC-SM-013, TC-SM-014, TC-SM-016 |  | Draft |
| FR-003 | HZ-003 | HLD-001 |  |  |  |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_record_inbound_message`, `rsrx_protocol_context_build_encode_request` | TC-PC-001, TC-PC-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_send_request_t`, `rsrx_transport_frame_t` | TC-TR-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame` | TC-CODEC-001, TC-CODEC-002, TC-CODEC-004 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event` | TC-API-005 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event` | TC-SUP-001, TC-SUP-004, TC-SUP-007, TC-SUP-010 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-004, TC-SM-005, TC-SM-011 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_build_encode_request`, `rsrx_protocol_context_clear_retransmission` | TC-PC-003 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t` | TC-TR-003 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame` | TC-CODEC-003, TC-CODEC-004, TC-CODEC-006 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event` | TC-API-006 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-001, TC-SUP-005 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event`, `rsrx_state_machine_reset` | TC-SM-006, TC-SM-010 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-003 | `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event`, `rsrx_orchestrator_reset` | TC-OR-002, TC-OR-003, TC-OR-005 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-001, TC-API-002 |  | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-001, TC-CFG-002, TC-CFG-003, TC-CFG-004, TC-CFG-005 |  | Draft |
| FR-007 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-002, TC-SM-016 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_codec_decode_frame` | TC-CODEC-002, TC-CODEC-005 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-006 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-003, TC-SM-013, TC-SM-017 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t`, `rsrx_transport_channel_state_t` | TC-TR-003 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_timer_expiry` | TC-API-007, TC-API-008 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_init`, `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event`, `rsrx_transport_supervisor_process_timer_expiry` | TC-SUP-002, TC-SUP-003, TC-SUP-004, TC-SUP-008, TC-SUP-009, TC-SUP-011, TC-SUP-012 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h`, `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` | TC-SM-007, TC-SM-008, TC-SM-012, TC-SM-015, TC-SM-017 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-005, TC-CFG-006 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_init`, `rsrx_protocol_context_build_encode_request` | TC-PC-004 |  | Draft |
| SR-004 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_platform_port_table_t` | TC-PLAT-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-002, TC-API-003, TC-API-006 |  | Draft |
| IF-001 |  | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` |  |  | Draft |
| IF-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state`, `rsrx_orchestrator_reset` | TC-OR-001, TC-OR-004, TC-OR-005 |  | Draft |
| IF-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h` | `rsrx_session_init`, `rsrx_session_start`, `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event`, `rsrx_session_process_timer_expiry`, `rsrx_session_get_state`, `rsrx_session_reset` | TC-API-001, TC-API-003, TC-API-004, TC-API-005, TC-API-006, TC-API-007, TC-API-008, TC-API-009 |  | Draft |
| IF-002 | HZ-007 | HLD-001 |  |  |  |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_clock_port_t`, `rsrx_timer_port_t`, `rsrx_diagnostics_port_t`, `rsrx_platform_port_table_t` | TC-PLAT-001, TC-PLAT-002 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_adapter_init`, `rsrx_platform_adapter_build_executor_table` | TC-PA-001 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_port_t` | TC-TR-001 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_timer_command_t`, `rsrx_diagnostic_record_t` | TC-PLAT-002, TC-PLAT-003 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_adapter_init`, `rsrx_transport_executor_dispatch`, `rsrx_platform_timer_executor_dispatch`, `rsrx_platform_adapter_init` | TC-PA-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_executor_dispatch` | TC-PA-002 |  | Draft |

## Notes

- 현재 단계에서는 요구사항, hazard, 상위 설계까지만 연결했다.
- 코드, 함수, 테스트, 리뷰 항목은 구현과 검토가 시작되면 채운다.
- 각 행은 삭제하지 말고 상태를 갱신하는 방식으로 유지한다.
