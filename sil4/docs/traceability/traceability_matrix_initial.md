# Initial Traceability Matrix

| Req ID | Safety ID | Design ID | Source File | Function | Test ID | Review Record | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FR-001 |  | HLD-001 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init` |  |  | Draft |
| FR-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state` | TC-OR-001 |  | Draft |
| FR-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_init`, `rsrx_session_start` | TC-API-001 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionSupervisorFlow` | TC-INT-001 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBoundedSoakPumpFlow` | TC-INT-009 |  | Draft |
| FR-002 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-001, TC-SM-002, TC-SM-009, TC-SM-013, TC-SM-014, TC-SM-016 |  | Draft |
| FR-003 | HZ-003 | HLD-001 |  |  |  |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_record_inbound_message`, `rsrx_protocol_context_resolve_inbound_event`, `rsrx_protocol_context_build_encode_request` | TC-PC-001, TC-PC-002, TC-PC-005, TC-PC-011, TC-PC-012, TC-PC-015, TC-PC-016, TC-PC-017 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_send_request_t`, `rsrx_transport_frame_t` | TC-TR-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame` | TC-CODEC-001, TC-CODEC-002, TC-CODEC-004 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event` | TC-API-005 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-012 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c`, `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_application_data_indication_t`, `vApplicationExecutorDispatch`, `rsrx_transport_adapter_get_last_inbound_message`, `rsrx_platform_adapter_build_executor_table` | TC-APP-001, TC-APP-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-013 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c`, `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_session_send_application_data`, `rsrx_transport_adapter_send_application_data` | TC-OUT-001, TC-OUT-004, TC-API-010, TC-PA-005 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event` | TC-SUP-001, TC-SUP-004, TC-SUP-007, TC-SUP-010, TC-SUP-015, TC-SUP-016, TC-SUP-017 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_init`, `rsrx_channel_manager_update_channel`, `rsrx_channel_manager_select_channel`, `rsrx_channel_manager_reset` | TC-CHM-001, TC-CHM-002, TC-CHM-004, TC-CHM-005, TC-CHM-006 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionSupervisorFlow` | TC-INT-001 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBoundedSoakPumpFlow` | TC-INT-009 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-010 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-011 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelRecoveryHoldoffFlow` | TC-INT-012 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryFlow` | TC-INT-014 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueOverflowRejectFlow` | TC-INT-015 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdEscalationFlow` | TC-INT-016 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInitialZeroSequenceProtocolErrorFlow` | TC-INT-028 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDuplicateInboundProtocolErrorFlow` | TC-INT-019 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInvalidConfirmationProtocolErrorFlow` | TC-INT-020 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFailoverInvalidConfirmationProtocolErrorFlow` | TC-INT-038 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRegressingConfirmationProtocolErrorFlow` | TC-INT-021 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFailoverRegressingConfirmationProtocolErrorFlow` | TC-INT-039 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow`, `vTestIntegratedRetransmissionFailoverTimeoutFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow` | TC-INT-033, TC-INT-034, TC-INT-036, TC-INT-041, TC-INT-054, TC-INT-055 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow` | TC-INT-037 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedReceiveErrorFailoverCarryoverFlow` | TC-INT-035 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFailoverTransientRecoveryFlow`, `vTestIntegratedFailoverTransientSoakFlow`, `vTestIntegratedChannelUpRefreshHoldoffFlow`, `vTestIntegratedChannelUpFlapResetFlow` | TC-INT-025, TC-INT-026, TC-INT-027, TC-INT-029 | RV-030 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureFailoverBudgetResetFlow` | TC-INT-040 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoverySendBudgetIsolationFlow` | TC-INT-042 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorCarryoverFlow` | TC-INT-043 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorResetFlow` | TC-INT-044 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBudgetScopeCloseoutFlow` | TC-INT-095 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryMixedTransientResetFlow` | TC-INT-045 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffSendBudgetIsolationFlow` | TC-INT-046 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackIsolationFlow` | TC-INT-056 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackRecoveryOrderingFlow` | TC-INT-057 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffReceiveErrorCarryoverFlow` | TC-INT-047 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffMixedTransientResetFlow` | TC-INT-048 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffFlapTransientAsymmetryFlow` | TC-INT-049 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-050 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-051 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientSoakFlow` | TC-INT-052 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunFlow` | TC-INT-058 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleCompletionFlow` | TC-INT-080 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleFeedbackFlow` | TC-INT-081 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleMixedFeedbackFlow` | TC-INT-082 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunRecoveryStaleMixedFeedbackFlow` | TC-INT-083 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyRecoveryStaleMixedFeedbackBudgetResetFlow` | TC-INT-084 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyRecoveryStaleMixedFeedbackBudgetResetLongRunFlow` | TC-INT-085 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpHoldoffTransientSoakFlow` | TC-INT-053 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackSoakFlow` | TC-INT-059 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionSoakFlow` | TC-INT-079 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapRecoveryFlow` | TC-INT-060 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapTimeoutFlow` | TC-INT-061 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-062 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleProtocolErrorFlow` | TC-INT-063 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapInvalidConfirmationProtocolErrorFlow` | TC-INT-064 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackTimeoutFlow` | TC-INT-065 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryFlow` | TC-INT-066 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-067 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackInvalidConfirmationProtocolErrorFlow` | TC-INT-068 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRegressingConfirmationProtocolErrorFlow` | TC-INT-069 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryOrderingFlow` | TC-INT-070 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRecoveryOrderingFlow` | TC-INT-071 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionTimeoutFlow` | TC-INT-072 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-076 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionInvalidConfirmationProtocolErrorFlow` | TC-INT-077 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRegressingConfirmationProtocolErrorFlow` | TC-INT-078 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedProtocolOrderingCloseoutFlow` | TC-INT-086 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryOrderingCloseoutFlow` | TC-INT-087 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapPostRecoveryOrderingFlow` | TC-INT-088 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryHeartbeatOrderingFlow` | TC-INT-093 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapPostRecoveryHeartbeatOrderingFlow` | TC-INT-094 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapUnconfirmedLatestRecoveryFlow` | TC-INT-089 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeOrderingCloseoutFlow` | TC-INT-090 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffFlapRuntimeOrderingCloseoutFlow` | TC-INT-091 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyPolicyCloseoutFlow` | TC-INT-092 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorBudgetScopeMatrix` | TC-SUP-022 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSendFeedbackOrderingMatrix` | TC-SUP-023 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorChannelEventOrderingMatrix` | TC-SUP-024 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionIsolationFlow` | TC-INT-073 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionRecoveryOrderingFlow` | TC-INT-074 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionBudgetResetFlow` | TC-INT-075 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-004, TC-SM-005, TC-SM-011 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_resolve_inbound_event`, `rsrx_protocol_context_build_encode_request`, `rsrx_protocol_context_clear_retransmission` | TC-PC-003, TC-PC-006, TC-PC-007, TC-PC-010, TC-PC-012, TC-PC-013, TC-PC-014, TC-PC-015, TC-PC-016, TC-PC-017 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-050 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-051 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-062 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleProtocolErrorFlow` | TC-INT-063 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapInvalidConfirmationProtocolErrorFlow` | TC-INT-064 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryFlow` | TC-INT-066 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-067 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackInvalidConfirmationProtocolErrorFlow` | TC-INT-068 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRegressingConfirmationProtocolErrorFlow` | TC-INT-069 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryOrderingFlow` | TC-INT-070 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRecoveryOrderingFlow` | TC-INT-071 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionTimeoutFlow` | TC-INT-072 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-076 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionInvalidConfirmationProtocolErrorFlow` | TC-INT-077 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRegressingConfirmationProtocolErrorFlow` | TC-INT-078 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedProtocolOrderingCloseoutFlow` | TC-INT-086 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryOrderingCloseoutFlow` | TC-INT-087 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapPostRecoveryOrderingFlow` | TC-INT-088 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryHeartbeatOrderingFlow` | TC-INT-093 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapPostRecoveryHeartbeatOrderingFlow` | TC-INT-094 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRepeatedGapUnconfirmedLatestRecoveryFlow` | TC-INT-089 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t` | TC-TR-003 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame` | TC-CODEC-003, TC-CODEC-004, TC-CODEC-006 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event` | TC-API-006 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-001, TC-SUP-005 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionRecoveryFlow` | TC-INT-002 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow`, `vTestIntegratedStaleRetransmissionProtocolErrorFlow`, `vTestIntegratedRepeatedGapRetransmissionFlow`, `vTestIntegratedRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionTimeoutFailSafeFlow` | TC-INT-022, TC-INT-023, TC-INT-030, TC-INT-031, TC-INT-032 | RV-030 | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow`, `vTestIntegratedRetransmissionFailoverTimeoutFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow`, `vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow` | TC-INT-033, TC-INT-034, TC-INT-036, TC-INT-037, TC-INT-041, TC-INT-054, TC-INT-055 | RV-030 | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event`, `rsrx_state_machine_reset` | TC-SM-006, TC-SM-010 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-003 | `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event`, `rsrx_orchestrator_reset` | TC-OR-002, TC-OR-003, TC-OR-005 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-001, TC-API-002 |  | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-001, TC-CFG-002, TC-CFG-003, TC-CFG-004, TC-CFG-005 |  | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-012 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-APP-003 |  | Draft |
| FR-007 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-002, TC-SM-016 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_codec_decode_frame` | TC-CODEC-002, TC-CODEC-005 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-006 |  | Draft |
| SR-001 | HZ-001 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDecodeFailureFlow` | TC-INT-005 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-003, TC-SM-013, TC-SM-017 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t`, `rsrx_transport_channel_state_t` | TC-TR-003 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_timer_expiry` | TC-API-007, TC-API-008 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_init`, `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event`, `rsrx_transport_supervisor_process_timer_expiry` | TC-SUP-002, TC-SUP-003, TC-SUP-004, TC-SUP-008, TC-SUP-009, TC-SUP-011, TC-SUP-012, TC-SUP-015, TC-SUP-016, TC-SUP-017 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSendFeedbackOrderingMatrix` | TC-SUP-023 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorChannelEventOrderingMatrix` | TC-SUP-024 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorTimerDelegationMatrix` | TC-SUP-025 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionRecoveryFlow` | TC-INT-002 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedTimeoutFailSafeFlow`, `vTestIntegratedChannelDownFailSafeFlow` | TC-INT-003, TC-INT-004 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureBudgetFlow` | TC-INT-006 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureBudgetResetFlow` | TC-INT-007 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-010 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedReceiveErrorBudgetFlow`, `vTestIntegratedReceiveErrorBudgetResetFlow`, `vTestIntegratedReceiveErrorFailoverCarryoverFlow`, `vTestIntegratedMixedTransientBudgetResetFlow` | TC-INT-017, TC-INT-018, TC-INT-024, TC-INT-035 | RV-030 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionSoakFlow` | TC-INT-079 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionTimeoutFailSafeFlow`, `vTestIntegratedRetransmissionFailoverRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow`, `vTestIntegratedRetransmissionFailoverTimeoutFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow` | TC-INT-032, TC-INT-033, TC-INT-034, TC-INT-036, TC-INT-041, TC-INT-054, TC-INT-055 | RV-030 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFailoverTransientRecoveryFlow`, `vTestIntegratedFailoverTransientSoakFlow`, `vTestIntegratedChannelUpRefreshHoldoffFlow`, `vTestIntegratedChannelUpFlapResetFlow` | TC-INT-025, TC-INT-026, TC-INT-027, TC-INT-029 | RV-030 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureFailoverBudgetResetFlow` | TC-INT-040 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoverySendBudgetIsolationFlow` | TC-INT-042 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorCarryoverFlow` | TC-INT-043 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorResetFlow` | TC-INT-044 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBudgetScopeCloseoutFlow` | TC-INT-095 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryMixedTransientResetFlow` | TC-INT-045 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffSendBudgetIsolationFlow` | TC-INT-046 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackIsolationFlow` | TC-INT-056 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackRecoveryOrderingFlow` | TC-INT-057 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffReceiveErrorCarryoverFlow` | TC-INT-047 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffMixedTransientResetFlow` | TC-INT-048 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffFlapTransientAsymmetryFlow` | TC-INT-049 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientSoakFlow` | TC-INT-052 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunFlow` | TC-INT-058 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleCompletionFlow` | TC-INT-080 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleFeedbackFlow` | TC-INT-081 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunStaleMixedFeedbackFlow` | TC-INT-082 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapTransientLongRunRecoveryStaleMixedFeedbackFlow` | TC-INT-083 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyRecoveryStaleMixedFeedbackBudgetResetFlow` | TC-INT-084 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyRecoveryStaleMixedFeedbackBudgetResetLongRunFlow` | TC-INT-085 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeOrderingCloseoutFlow` | TC-INT-090 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpHoldoffTransientSoakFlow` | TC-INT-053 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleFeedbackSoakFlow` | TC-INT-059 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapRecoveryFlow` | TC-INT-060 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapTimeoutFlow` | TC-INT-061 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-062 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleProtocolErrorFlow` | TC-INT-063 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapInvalidConfirmationProtocolErrorFlow` | TC-INT-064 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackTimeoutFlow` | TC-INT-065 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryFlow` | TC-INT-066 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-067 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackInvalidConfirmationProtocolErrorFlow` | TC-INT-068 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRegressingConfirmationProtocolErrorFlow` | TC-INT-069 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-076 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionInvalidConfirmationProtocolErrorFlow` | TC-INT-077 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRegressingConfirmationProtocolErrorFlow` | TC-INT-078 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleFeedbackRecoveryOrderingFlow` | TC-INT-070 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionRecoveryOrderingFlow` | TC-INT-071 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionChannelUpHoldoffRepeatedGapStaleCompletionTimeoutFlow` | TC-INT-072 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionIsolationFlow` | TC-INT-073 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionRecoveryOrderingFlow` | TC-INT-074 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionBudgetResetFlow` | TC-INT-075 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-050 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-051 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h`, `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` | TC-SM-007, TC-SM-008, TC-SM-012, TC-SM-015, TC-SM-017 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-005, TC-CFG-006 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_init`, `rsrx_protocol_context_build_encode_request` | TC-PC-004 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_init`, `rsrx_channel_manager_select_channel` | TC-CHM-003, TC-CHM-006 |  | Draft |
| SR-003 | HZ-005 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInitialZeroSequenceProtocolErrorFlow`, `vTestIntegratedDuplicateInboundProtocolErrorFlow`, `vTestIntegratedInvalidConfirmationProtocolErrorFlow`, `vTestIntegratedFailoverInvalidConfirmationProtocolErrorFlow`, `vTestIntegratedRegressingConfirmationProtocolErrorFlow`, `vTestIntegratedFailoverRegressingConfirmationProtocolErrorFlow`, `vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow`, `vTestIntegratedStaleRetransmissionProtocolErrorFlow`, `vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow` | TC-INT-019, TC-INT-020, TC-INT-021, TC-INT-022, TC-INT-023, TC-INT-028, TC-INT-037, TC-INT-038, TC-INT-039 | RV-030 | Draft |
| SR-004 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_platform_port_table_t` | TC-PLAT-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-002, TC-API-003, TC-API-006 |  | Draft |
| IF-001 |  | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` |  |  | Draft |
| IF-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state`, `rsrx_orchestrator_reset` | TC-OR-001, TC-OR-004, TC-OR-005 |  | Draft |
| IF-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h` | `rsrx_session_init`, `rsrx_session_start`, `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event`, `rsrx_session_process_timer_expiry`, `rsrx_session_get_state`, `rsrx_session_reset` | TC-API-001, TC-API-003, TC-API-004, TC-API-005, TC-API-006, TC-API-007, TC-API-008, TC-API-009 |  | Draft |
| IF-001 |  | HLD-001, LLD-013 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_send_application_data` | TC-OUT-001, TC-OUT-002, TC-OUT-003, TC-API-010, TC-API-011 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryFlow` | TC-INT-014 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueOverflowRejectFlow` | TC-INT-015 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdEscalationFlow` | TC-INT-016 |  | Draft |
| IF-002 | HZ-007 | HLD-001 |  |  |  |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_clock_port_t`, `rsrx_timer_port_t`, `rsrx_diagnostics_port_t`, `rsrx_platform_port_table_t` | TC-PLAT-001, TC-PLAT-002 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_adapter_init`, `rsrx_platform_adapter_build_executor_table`, `rsrx_transport_adapter_query_channel` | TC-PA-001, TC-PA-006 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_port_t` | TC-TR-001 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_config_t`, `rsrx_channel_selection_result_t` | TC-CHM-001, TC-CHM-005, TC-CHM-006 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_timer_command_t`, `rsrx_diagnostic_record_t` | TC-PLAT-002, TC-PLAT-003 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_adapter_init`, `rsrx_transport_executor_dispatch`, `rsrx_platform_timer_executor_dispatch`, `rsrx_platform_adapter_init` | TC-PA-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_adapter_record_inbound_message`, `rsrx_transport_adapter_get_last_inbound_message`, `rsrx_transport_adapter_query_channel`, `rsrx_platform_adapter_build_executor_table` | TC-PA-001, TC-PA-004, TC-PA-006, TC-PA-007, TC-PA-008 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelRecoveryHoldoffFlow` | TC-INT-012 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |

## Notes

- 현재 단계에서는 요구사항, hazard, 상위 설계까지만 연결했다.
- 코드, 함수, 테스트, 리뷰 항목은 구현과 검토가 시작되면 채운다.
- 각 행은 삭제하지 말고 상태를 갱신하는 방식으로 유지한다.
