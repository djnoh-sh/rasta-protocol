# Initial Traceability Matrix

| Req ID | Safety ID | Design ID | Source File | Function | Test ID | Review Record | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FR-001 |  | HLD-001 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init` |  |  | Draft |
| FR-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state` | TC-OR-001 |  | Draft |
| FR-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_init`, `rsrx_session_start` | TC-API-001, TC-API-016, TC-API-018 | RV-392, RV-396 | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionSupervisorFlow` | TC-INT-001 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBoundedSoakPumpFlow` | TC-INT-009 |  | Draft |
| FR-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionRestartAfterResetFlow` | TC-INT-209 | RV-397 | Draft |
| FR-002 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event`, `rsrx_state_machine_reset` | TC-SM-001, TC-SM-002, TC-SM-009, TC-SM-013, TC-SM-014, TC-SM-016, TC-SM-018 | RV-393 | Draft |
| FR-003 | HZ-003 | HLD-001 |  |  |  |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_init`, `rsrx_protocol_context_record_inbound_message`, `rsrx_protocol_context_resolve_inbound_event`, `rsrx_protocol_context_build_encode_request` | TC-PC-001, TC-PC-002, TC-PC-005, TC-PC-011, TC-PC-012, TC-PC-015, TC-PC-016, TC-PC-017, TC-PC-018, TC-PC-019, TC-PC-020, TC-PC-021, TC-PC-022, TC-PC-023, TC-PC-024, TC-PC-025, TC-PC-026, TC-PC-028, TC-PC-029, TC-PC-030, TC-PC-031 | RV-217, RV-263, RV-266, RV-270, RV-275, RV-278, RV-281, RV-282, RV-292, RV-391, RV-419, RV-420, RV-421 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_send_request_t`, `rsrx_transport_frame_t` | TC-TR-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_port_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame`, `rsrx_codec_get_default_port`, `rsrx_codec_get_rasta_sr_default_checksum_profile`, `rsrx_codec_get_rasta_redundancy_wire_profile`, `rsrx_codec_validate_rasta_redundancy_crc_profile`, `rsrx_codec_encode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum` | TC-CODEC-001, TC-CODEC-002, TC-CODEC-004, TC-CODEC-005, TC-CODEC-007, TC-CODEC-008, TC-CODEC-009, TC-CODEC-010, TC-CODEC-011, TC-CODEC-012, TC-CODEC-013, TC-CODEC-014, TC-CODEC-015, TC-CODEC-016, TC-CODEC-017, TC-CODEC-018, TC-CODEC-019, TC-CODEC-020, TC-CODEC-021, TC-CODEC-022, TC-CODEC-023, TC-CODEC-029, TC-CODEC-031, TC-CODEC-033, TC-CODEC-034, TC-CODEC-035, TC-CODEC-036, TC-CODEC-045, TC-CODEC-046, TC-CODEC-047, TC-CODEC-048, TC-CODEC-049, TC-CODEC-050 | RV-305, RV-306, RV-307, RV-308, RV-309, RV-310, RV-311, RV-312, RV-313, RV-314, RV-315, RV-316, RV-317, RV-318, RV-319, RV-320, RV-321, RV-337, RV-347, RV-348, RV-349, RV-350, RV-351, RV-352, RV-353, RV-369, RV-370, RV-371, RV-372, RV-373, RV-374, RV-375, RV-376, RV-377, RV-378, RV-382, RV-383, RV-384, RV-390, RV-409, RV-411, RV-412, RV-413, RV-414, RV-416 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event`, `rsrx_session_reset` | TC-API-005, TC-API-012, TC-API-013, TC-API-017, TC-API-018 | RV-296, RV-297, RV-298, RV-302, RV-395, RV-396 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-012 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c`, `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_application_data_indication_t`, `vApplicationExecutorDispatch`, `rsrx_transport_adapter_get_last_inbound_message`, `rsrx_platform_adapter_build_executor_table` | TC-APP-001, TC-APP-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-013 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c`, `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_session_send_application_data`, `rsrx_transport_adapter_send_application_data` | TC-OUT-001, TC-OUT-004, TC-OUT-005, TC-OUT-006, TC-OUT-008, TC-OUT-009, TC-API-010, TC-PA-005 | RV-233 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_init`, `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event`, `rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime` | TC-SUP-001, TC-SUP-004, TC-SUP-007, TC-SUP-010, TC-SUP-015, TC-SUP-016, TC-SUP-017, TC-SUP-038, TC-SUP-064, TC-SUP-065, TC-SUP-066, TC-SUP-067, TC-SUP-068, TC-SUP-069, TC-SUP-071, TC-SUP-073, TC-SUP-074, TC-SUP-075, TC-SUP-076, TC-SUP-077, TC-INT-187, TC-INT-210 | RV-272, RV-279, RV-280, RV-283, RV-284, RV-285, RV-286, RV-287, RV-288, RV-289, RV-290, RV-294, RV-300, RV-324, RV-388, RV-389, RV-408, RV-410, RV-417, RV-418 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_init`, `rsrx_channel_manager_update_channel`, `rsrx_channel_manager_select_channel`, `rsrx_channel_manager_get_active_channel`, `rsrx_channel_manager_reset` | TC-CHM-001, TC-CHM-002, TC-CHM-004, TC-CHM-005, TC-CHM-006, TC-CHM-007, TC-CHM-008, TC-CHM-009, TC-CHM-010, TC-CHM-011, TC-CHM-053, TC-CHM-054, TC-CHM-055, TC-CHM-056, TC-CHM-057, TC-CHM-058, TC-CHM-059, TC-CHM-060, TC-CHM-061 | RV-283, RV-284, RV-285, RV-286, RV-287, RV-288, RV-289, RV-290, RV-295, RV-368, RV-379, RV-381, RV-387 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestChannelManagerRejectsInvalidTopologyConfig` | TC-CHM-049 | RV-262 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoff` | TC-CHM-052 | RV-274 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyHoldoff` | TC-CHM-053 | RV-283, RV-284, RV-285, RV-286 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyBypassClear` | TC-CHM-054 | RV-287 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyAppliedCycleCount` | TC-CHM-055 | RV-288 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyAbortCount` | TC-CHM-056 | RV-289, RV-290 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyResetClearCount` | TC-CHM-057 | RV-295 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryFlapPenaltyTargetSaturates` | TC-CHM-060 | RV-381 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionSupervisorFlow` | TC-INT-001 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBoundedSoakPumpFlow` | TC-INT-009 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-010 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-011 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelRecoveryHoldoffFlow` | TC-INT-012 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffActiveLossBypassFlow` | TC-INT-104 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffActiveLossBypassLongRunFlow` | TC-INT-105 | RV-251 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedActiveLossBypassReentersHoldoffFlow` | TC-INT-106 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapResetThenActiveLossBypassFlow` | TC-INT-107 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapResetThenActiveLossBypassLongRunFlow` | TC-INT-108 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassMixedTransientLongRunFlow` | TC-INT-109 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleFeedbackLongRunFlow` | TC-INT-110 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleMixedFeedbackBudgetResetLongRunFlow` | TC-INT-193 | RV-253 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleCompletionLongRunFlow` | TC-INT-112 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleCompletionBudgetResetLongRunFlow` | TC-INT-194 | RV-254 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleFeedbackBudgetResetLongRunFlow` | TC-INT-195 | RV-255 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpFlapPenaltyFlow` | TC-INT-201 | RV-283, RV-284, RV-285, RV-286 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpFlapPenaltyBypassClearFlow` | TC-INT-202 | RV-287 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpFlapPenaltyAppliedCycleFlow` | TC-INT-203 | RV-288 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelUpFlapPenaltyAbortFlow` | TC-INT-204 | RV-289, RV-290 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveCarryoverFlow` | TC-INT-113 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveResetFlow` | TC-INT-114 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveCarryoverResetLongRunFlow` | TC-INT-192 | RV-252 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryFlow` | TC-INT-014 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionResetOutboundTelemetryFlow` | TC-INT-205 | RV-303 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionRestartAfterResetFlow` | TC-INT-209 | RV-397 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInvalidChannelDecodeFailureFlow` | TC-INT-206 | RV-322, RV-353 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedNonFrameReceiveNoFrameFlow` | TC-INT-207 | RV-323 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRastaRedundancySrRuntimeFlow` | TC-INT-210 | RV-418 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveNonFrameNoFrameGating` | TC-SUP-071 | RV-324 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueFifoDispatchFlow` | TC-INT-102 | RV-034 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueMixedClearOrderingFlow` | TC-INT-103 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueMixedClearLongRunFlow` | TC-INT-116 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestApplicationDataDeferredQueueMixedClearLongRun` | TC-OUT-010 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestBusyRejectThresholdManualInboundResetSources` | TC-OUT-011 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestDeferredQueueTelemetryAccumulationMatrix` | TC-OUT-012 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOverflowBusyAccumulationMatrix` | TC-OUT-013 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-013 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestApplicationDataSend` | TC-OUT-017 | RV-273 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueLongRunRepresentativeMatrix` | TC-OUT-015 | RV-228 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueFairnessRepresentativeMatrix` | TC-OUT-016 | RV-230 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueBackpressureCloseoutMatrix` | TC-OUT-014 | RV-242 | Draft |
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
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyHysteresisCloseoutFlow` | TC-INT-096 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyLongRunCloseoutFlow` | TC-INT-097 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassCloseoutFlow` | TC-INT-111 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedProtocolVariantCloseoutFlow` | TC-INT-098 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedConnectResponseSequencingRepresentativeFlow` | TC-INT-181 | RV-215 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedNonDataSequencedMessageRepresentativeFlow` | TC-INT-182 | RV-216 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeCorrelatedFeedbackRepresentativeFlow` | TC-INT-183 | RV-219 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeStaleFeedbackLongRunRepresentativeFlow` | TC-INT-184 | RV-221 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueLongRunRepresentativeFlow` | TC-INT-185 | RV-224 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryRetransmissionRequestOrderingFlow` | TC-INT-099 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueBackpressureCloseoutFlow` | TC-INT-100 | RV-242 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdResetFlow` | TC-INT-101 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdInboundResetFlow` | TC-INT-115 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectAlternatingResetFlow` | TC-INT-117 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryAccumulationFlow` | TC-INT-118 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueOverflowAccumulationFlow` | TC-INT-119 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedOverflowBusyAccumulationFlow` | TC-INT-120 |  | Draft |
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
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditCloseoutMatrix` | TC-SUP-034 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditCumulativeMatrix` | TC-SUP-035 | RV-107 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditEnvelopeMatrix` | TC-SUP-036 | RV-109 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditReasonMatrix` | TC-SUP-037 | RV-121 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorChannelEventOrderingMatrix` | TC-SUP-037 | RV-124 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditHoldoffProgressMatrix` | TC-SUP-038 | RV-126 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditHoldoffResetMatrix` | TC-SUP-039 | RV-127 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditFlapPenaltyMatrix` | TC-SUP-065 | RV-283, RV-284, RV-285, RV-286 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditFlapPenaltyBypassClearMatrix` | TC-SUP-066 | RV-287 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditFlapPenaltyAppliedCycleMatrix` | TC-SUP-067 | RV-288 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditFlapPenaltyAbortMatrix` | TC-SUP-068 | RV-289, RV-290 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditEnvelopeMatrix` | TC-SUP-040 | RV-128, RV-294 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeEnvelopeMatrix` | TC-SUP-043 | RV-150 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeStabilityLongRunMatrix` | TC-SUP-049 | RV-168 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdEightMixedLongRunMatrix` | TC-SUP-050 | RV-171 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdNineMixedLongRunMatrix` | TC-SUP-051 | RV-172 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdTenMixedLongRunMatrix` | TC-SUP-052 | RV-173 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdElevenMixedLongRunMatrix` | TC-SUP-053 | RV-177 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdTwelveMixedLongRunMatrix` | TC-SUP-054 | RV-180 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdThirteenMixedLongRunMatrix` | TC-SUP-055 | RV-181 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdFourteenMixedLongRunMatrix` | TC-SUP-056 | RV-182 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdFifteenMixedLongRunMatrix` | TC-SUP-057 | RV-183 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorRuntimeOrderingCloseoutMatrix` | TC-SUP-032 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorReportExposesBusyRejectTelemetry` | TC-SUP-033 | RV-273 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorReportExposesRuntimeResetTelemetry` | TC-SUP-069 | RV-300 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorQueueReportMatrix` | TC-SUP-059 | RV-226, RV-300 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryStabilityEnvelopeCloseoutMatrix` | TC-CHM-017 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestChannelManagerRejectsRuntimeTopologyMutation` | TC-CHM-050 | RV-267 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestChannelManagerRejectsDuplicatePriorityTopology` | TC-CHM-051 | RV-271 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-014 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestChannelManagerRejectsUnsupportedRedundantChannelTopology` | TC-CHM-058 | RV-368 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestChannelManagerQueryRejectsTopologyMutation` | TC-PA-009 | RV-268 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestTransportAdapterRuntimeReset` | TC-PA-010 | RV-299, RV-301 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedTopologyMismatchQueryBudgetFlow` | TC-INT-200 | RV-269 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestRetransmissionBaseWrapRejected` | TC-PC-022 | RV-270 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestEncodeFailureClearsRequest` | TC-PC-029 | RV-419 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestOutboundPayloadPointerGuard` | TC-PC-030 | RV-420 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestInvalidSequenceRecordRejected` | TC-PC-031 | RV-421 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestUnsequencedMessageFamilyPassThroughMatrix` | TC-PC-019 | RV-275 | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-011 | `sil4/tests/unit/test_rsrx_protocol_context.c` | `vTestConnectRequestRemainsUnsequencedBaseline` | TC-PC-026 | RV-292 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFive` | TC-CHM-018 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFiveFlapReset` | TC-CHM-019 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSix` | TC-CHM-020 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSixFlapReset` | TC-CHM-021 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSeven` | TC-CHM-022 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSevenFlapReset` | TC-CHM-023 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdEight` | TC-CHM-024 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdEightFlapReset` | TC-CHM-025 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdNine` | TC-CHM-026 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdNineFlapReset` | TC-CHM-027 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdTen` | TC-CHM-028 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdTenFlapReset` | TC-CHM-029 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdEleven` | TC-CHM-030 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdElevenFlapReset` | TC-CHM-031 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdTwelve` | TC-CHM-032 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdTwelveFlapReset` | TC-CHM-033 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdThirteen` | TC-CHM-034 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdThirteenFlapReset` | TC-CHM-035 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFourteen` | TC-CHM-036 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFourteenFlapReset` | TC-CHM-037 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFifteen` | TC-CHM-038 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdFifteenFlapReset` | TC-CHM-039 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSixteen` | TC-CHM-040 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSixteenFlapReset` | TC-CHM-041 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSeventeen` | TC-CHM-042 | RV-246 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdSeventeenFlapReset` | TC-CHM-043 | RV-247 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdEighteen` | TC-CHM-044 | RV-248 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdEighteenFlapReset` | TC-CHM-045 | RV-249 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdNineteen` | TC-CHM-046 | RV-259 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdNineteenFlapReset` | TC-CHM-047 | RV-259 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_channel_manager.c` | `vTestPreferredRecoveryHoldoffThresholdTwenty` | TC-CHM-048 | RV-260 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c`, `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_outbound_send_telemetry_t::uMaxDeferredSendCount`, `rsrx_transport_supervisor_report_t::uMaxDeferredSendCount`, `rsrx_transport_adapter_send_application_data` | TC-OUT-012, TC-SUP-019, TC-INT-014, TC-INT-118 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionIsolationFlow` | TC-INT-073 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionRecoveryOrderingFlow` | TC-INT-074 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionBudgetResetFlow` | TC-INT-075 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-004, TC-SM-005, TC-SM-011 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_init`, `rsrx_protocol_context_resolve_inbound_event`, `rsrx_protocol_context_build_encode_request`, `rsrx_protocol_context_clear_retransmission` | TC-PC-003, TC-PC-006, TC-PC-007, TC-PC-010, TC-PC-012, TC-PC-013, TC-PC-014, TC-PC-015, TC-PC-016, TC-PC-017, TC-PC-018, TC-PC-022, TC-PC-027, TC-PC-028, TC-PC-029 | RV-270, RV-386, RV-391, RV-419 | Draft |
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
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedProtocolVariantCloseoutFlow` | TC-INT-098 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedNonDataSequencedMessageRepresentativeFlow` | TC-INT-182 | RV-216 | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPostRecoveryRetransmissionRequestOrderingFlow` | TC-INT-099 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t` | TC-TR-003 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_encode_request_t`, `rsrx_encode_buffer_t`, `rsrx_codec_port_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame`, `rsrx_codec_get_default_port`, `rsrx_codec_get_rasta_sr_default_checksum_profile`, `rsrx_codec_get_rasta_redundancy_wire_profile`, `rsrx_codec_validate_rasta_redundancy_crc_profile`, `rsrx_codec_encode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum` | TC-CODEC-003, TC-CODEC-004, TC-CODEC-006, TC-CODEC-008, TC-CODEC-012, TC-CODEC-014, TC-CODEC-015, TC-CODEC-016, TC-CODEC-018, TC-CODEC-020, TC-CODEC-021, TC-CODEC-029, TC-CODEC-031, TC-CODEC-033, TC-CODEC-034, TC-CODEC-035, TC-CODEC-036, TC-CODEC-045, TC-CODEC-046, TC-CODEC-047, TC-CODEC-048, TC-CODEC-049, TC-CODEC-050 | RV-306, RV-310, RV-312, RV-313, RV-314, RV-316, RV-318, RV-319, RV-337, RV-349, RV-351, RV-369, RV-370, RV-373, RV-382, RV-383, RV-384, RV-390, RV-409, RV-411, RV-412, RV-413, RV-414, RV-416 | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_event` | TC-API-006 |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-001, TC-SUP-005 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionRecoveryFlow` | TC-INT-002 |  | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow`, `vTestIntegratedStaleRetransmissionProtocolErrorFlow`, `vTestIntegratedRepeatedGapRetransmissionFlow`, `vTestIntegratedRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionTimeoutFailSafeFlow` | TC-INT-022, TC-INT-023, TC-INT-030, TC-INT-031, TC-INT-032 | RV-030 | Draft |
| FR-004 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow`, `vTestIntegratedRetransmissionFailoverTimeoutFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow`, `vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow` | TC-INT-033, TC-INT-034, TC-INT-036, TC-INT-037, TC-INT-041, TC-INT-054, TC-INT-055 | RV-030 | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event`, `rsrx_state_machine_reset` | TC-SM-006, TC-SM-010 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-003 | `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event`, `rsrx_orchestrator_reset` | TC-OR-002, TC-OR-003, TC-OR-005, TC-OR-006 | RV-394 | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-001, TC-API-002 |  | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-001, TC-CFG-002, TC-CFG-003, TC-CFG-004, TC-CFG-005, TC-CFG-007, TC-CFG-008 | RV-265, RV-271 | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-010 | `sil4/tests/unit/test_rsrx_config_validator.c` | `vTestDuplicateChannelPriorityRejected` | TC-CFG-008 | RV-271 | Draft |
| FR-006 | HZ-004 | HLD-001, LLD-012 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-APP-003 |  | Draft |
| FR-007 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| FR-007 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-002, TC-SM-016 |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-008 | `sil4/include/rsrx_codec.h`, `sil4/src/rsrx_codec.c` | `rsrx_decoded_message_t`, `rsrx_codec_encode_message`, `rsrx_codec_decode_frame`, `rsrx_codec_get_rasta_sr_default_checksum_profile`, `rsrx_codec_get_rasta_redundancy_wire_profile`, `rsrx_codec_validate_rasta_redundancy_crc_profile`, `rsrx_codec_encode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_no_crc`, `rsrx_codec_decode_rasta_redundancy_carried_sr_no_checksum` | TC-CODEC-002, TC-CODEC-005, TC-CODEC-007, TC-CODEC-008, TC-CODEC-009, TC-CODEC-010, TC-CODEC-011, TC-CODEC-013, TC-CODEC-016, TC-CODEC-017, TC-CODEC-019, TC-CODEC-020, TC-CODEC-022, TC-CODEC-023, TC-CODEC-029, TC-CODEC-031, TC-CODEC-033, TC-CODEC-034, TC-CODEC-036, TC-CODEC-045, TC-CODEC-046, TC-CODEC-047, TC-CODEC-048, TC-CODEC-049, TC-CODEC-050 | RV-305, RV-306, RV-307, RV-308, RV-309, RV-311, RV-314, RV-315, RV-317, RV-318, RV-320, RV-321, RV-337, RV-347, RV-348, RV-349, RV-350, RV-351, RV-352, RV-353, RV-369, RV-370, RV-371, RV-372, RV-373, RV-374, RV-375, RV-376, RV-377, RV-378, RV-382, RV-383, RV-390, RV-409, RV-411, RV-412, RV-413, RV-414, RV-416 | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_process_frame` | TC-SUP-006 |  | Draft |
| SR-001 | HZ-001 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDecodeFailureFlow`, `vTestIntegratedInvalidChannelDecodeFailureFlow` | TC-INT-005, TC-INT-206 | RV-322, RV-353 | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-003, TC-SM-013, TC-SM-017 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_frame_t`, `rsrx_transport_channel_state_t` | TC-TR-003 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_process_timer_expiry` | TC-API-007, TC-API-008 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-009 | `sil4/include/rsrx_transport_supervisor.h`, `sil4/src/rsrx_transport_supervisor.c` | `rsrx_transport_supervisor_init`, `rsrx_transport_supervisor_process_frame`, `rsrx_transport_supervisor_poll_receive`, `rsrx_transport_supervisor_process_transport_event`, `rsrx_transport_supervisor_process_timer_expiry`, `rsrx_transport_supervisor_pump_receive`, `rsrx_transport_supervisor_enable_rasta_redundancy_sr_runtime` | TC-SUP-002, TC-SUP-003, TC-SUP-004, TC-SUP-008, TC-SUP-009, TC-SUP-011, TC-SUP-012, TC-SUP-014, TC-SUP-015, TC-SUP-016, TC-SUP-017, TC-SUP-070, TC-SUP-071, TC-SUP-072, TC-SUP-073, TC-SUP-074, TC-SUP-075, TC-SUP-076, TC-SUP-077 | RV-304, RV-324, RV-325, RV-326, RV-380, RV-388, RV-389, RV-408, RV-410, RV-417 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveChannelDown` | TC-SUP-008 | RV-326 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveNoFrame` | TC-SUP-008 | RV-325 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorInitClearsReportBaseline` | TC-SUP-070 | RV-304 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSendFeedbackOrderingMatrix` | TC-SUP-023 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorChannelEventOrderingMatrix` | TC-SUP-024 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditCloseoutMatrix` | TC-SUP-034 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditCumulativeMatrix` | TC-SUP-035 | RV-107 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditEnvelopeMatrix` | TC-SUP-036 | RV-109 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditReasonMatrix` | TC-SUP-037 | RV-121 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorChannelEventOrderingMatrix` | TC-SUP-037 | RV-124 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditHoldoffProgressMatrix` | TC-SUP-038 | RV-126 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditHoldoffResetMatrix` | TC-SUP-039 | RV-127 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditEnvelopeMatrix` | TC-SUP-040 | RV-128, RV-294 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorTimerDelegationMatrix` | TC-SUP-025 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveRetryOrderingMatrix` | TC-SUP-026 | RV-277 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPumpReceiveTerminalOrderingMatrix` | TC-SUP-027 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPumpReceiveErrorOrderingMatrix` | TC-SUP-028 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPumpReceiveIgnoredOrderingMatrix` | TC-SUP-029 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPumpReceiveEscalationOrderingMatrix` | TC-SUP-030 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPumpReceiveMaxPollOrderingMatrix` | TC-SUP-031 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorRuntimeOrderingCloseoutMatrix` | TC-SUP-032 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorTransportFaultEscalationDecisionMatrix` | TC-SUP-063 | RV-264 | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-009 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveErrorBudgeted`, `vTestSupervisorPollQueryErrorStageTelemetry`, `vTestSupervisorPollReceiveErrorEscalatesAndResets`, `vTestSupervisorPollReceiveRetryOrderingMatrix` | TC-SUP-064 | RV-272, RV-276 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionRecoveryFlow` | TC-INT-002 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedTimeoutFailSafeFlow`, `vTestIntegratedChannelDownFailSafeFlow` | TC-INT-003, TC-INT-004 | RV-264 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureBudgetFlow` | TC-INT-006 | RV-264 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureBudgetResetFlow` | TC-INT-007 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelFailoverFlow` | TC-INT-010 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedReceiveErrorBudgetFlow`, `vTestIntegratedTopologyMismatchQueryBudgetFlow`, `vTestIntegratedReceiveErrorBudgetResetFlow`, `vTestIntegratedReceiveErrorFailoverCarryoverFlow`, `vTestIntegratedMixedTransientBudgetResetFlow` | TC-INT-017, TC-INT-200, TC-INT-018, TC-INT-024, TC-INT-035 | RV-030, RV-269 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInvalidChannelDecodeFailureFlow` | TC-INT-206 | RV-322, RV-353 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedNonFrameReceiveNoFrameFlow` | TC-INT-207 | RV-323 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorPollReceiveNonFrameNoFrameGating` | TC-SUP-071 | RV-324 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffStaleCompletionSoakFlow` | TC-INT-079 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionTimeoutFailSafeFlow`, `vTestIntegratedRetransmissionFailoverRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffRecoveryFlow`, `vTestIntegratedRetransmissionChannelUpHoldoffTimeoutFlow`, `vTestIntegratedRetransmissionFailoverTimeoutFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapRecoveryFlow`, `vTestIntegratedRetransmissionFailoverRepeatedGapTimeoutFlow` | TC-INT-032, TC-INT-033, TC-INT-034, TC-INT-036, TC-INT-041, TC-INT-054, TC-INT-055 | RV-030 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFailoverTransientRecoveryFlow`, `vTestIntegratedFailoverTransientSoakFlow`, `vTestIntegratedChannelUpRefreshHoldoffFlow`, `vTestIntegratedChannelUpFlapResetFlow` | TC-INT-025, TC-INT-026, TC-INT-027, TC-INT-029 | RV-030 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSendFailureFailoverBudgetResetFlow` | TC-INT-040 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoverySendBudgetIsolationFlow` | TC-INT-042 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorCarryoverFlow` | TC-INT-043 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryReceiveErrorResetFlow` | TC-INT-044 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBudgetScopeCloseoutFlow` | TC-INT-095 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeCorrelatedFeedbackRepresentativeFlow` | TC-INT-183 | RV-219 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRuntimeStaleFeedbackLongRunRepresentativeFlow` | TC-INT-184 | RV-221 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyHysteresisCloseoutFlow` | TC-INT-096 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyLongRunCloseoutFlow` | TC-INT-097 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassCloseoutFlow` | TC-INT-111 |  | Draft |
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
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffActiveLossBypassFlow` | TC-INT-104 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedHoldoffActiveLossBypassLongRunFlow` | TC-INT-105 | RV-251 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedActiveLossBypassReentersHoldoffFlow` | TC-INT-106 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorReportExposesBusyRejectTelemetry` | TC-SUP-033 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorReportExposesRuntimeResetTelemetry` | TC-SUP-069 | RV-300 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapResetThenActiveLossBypassFlow` | TC-INT-107 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapResetThenActiveLossBypassLongRunFlow` | TC-INT-108 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassMixedTransientLongRunFlow` | TC-INT-109 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleFeedbackLongRunFlow` | TC-INT-110 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleMixedFeedbackBudgetResetLongRunFlow` | TC-INT-193 | RV-253 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleCompletionLongRunFlow` | TC-INT-112 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleCompletionBudgetResetLongRunFlow` | TC-INT-194 | RV-254 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassStaleFeedbackBudgetResetLongRunFlow` | TC-INT-195 | RV-255 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveCarryoverFlow` | TC-INT-113 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveResetFlow` | TC-INT-114 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedFlapBypassReceiveCarryoverResetLongRunFlow` | TC-INT-192 | RV-252 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdThreeFlow` | TC-INT-121 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFourFlow` | TC-INT-122 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFiveFlow` | TC-INT-129 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSixFlow` | TC-INT-132 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSevenFlow` | TC-INT-135 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdEightFlow` | TC-INT-137 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdNineFlow` | TC-INT-139 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdTenFlow` | TC-INT-141 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdElevenFlow` | TC-INT-143 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdTwelveFlow` | TC-INT-145 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdThirteenFlow` | TC-INT-147 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFourteenFlow` | TC-INT-149 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFifteenFlow` | TC-INT-151 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSixteenFlow` | TC-INT-153 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSeventeenFlow` | TC-INT-187 | RV-246 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdEighteenFlow` | TC-INT-189 | RV-248 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdNineteenFlow` | TC-INT-197 | RV-258 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdTwentyFlow` | TC-INT-199 | RV-260 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditLongRunFlow` | TC-INT-123 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditCloseoutFlow` | TC-INT-127 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditCumulativeFlow` | TC-INT-131 | RV-107 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditEnvelopeFlow` | TC-INT-134 | RV-109 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditReasonFlow` | TC-INT-155 | RV-121 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditLongRunFlow` | TC-INT-155 | RV-124 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditHoldoffProgressFlow` | TC-INT-156 | RV-126 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditHoldoffResetFlowWrapper` | TC-INT-157 | RV-127 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditEnvelopeFlow` | TC-INT-158 | RV-128, RV-294 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeEnvelopeFlow` | TC-INT-161 | RV-150 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeStabilityLongRunFlow` | TC-INT-167 | RV-168 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdEightMixedLongRunFlow` | TC-INT-168 | RV-171 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdNineMixedLongRunFlow` | TC-INT-169 | RV-172 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdTenMixedLongRunFlow` | TC-INT-170 | RV-173 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdElevenMixedLongRunFlow` | TC-INT-174 | RV-177 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdTwelveMixedLongRunFlow` | TC-INT-175 | RV-180 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdThirteenMixedLongRunFlow` | TC-INT-176 | RV-181 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdFourteenMixedLongRunFlow` | TC-INT-177 | RV-182 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdFifteenMixedLongRunFlow` | TC-INT-178 | RV-183 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFeedbackLongRunRepresentativeFlow` | TC-INT-172 | RV-175 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyBypassReentryRepresentativeFlow` | TC-INT-173 | RV-176 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapBypassReceiveRepresentativeFlow` | TC-INT-180 | RV-187 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyStabilityLongRunRepresentativeFlow` | TC-INT-171 | RV-174, RV-256 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyStabilityEnvelopeCloseoutFlow` | TC-INT-128 | RV-166 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdThreeFlapResetFlow` | TC-INT-124 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFourFlapResetFlow` | TC-INT-125 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFiveFlapResetFlow` | TC-INT-130 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSixFlapResetFlow` | TC-INT-133 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSevenFlapResetFlow` | TC-INT-136 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdEightFlapResetFlow` | TC-INT-138 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdNineFlapResetFlow` | TC-INT-140 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdTenFlapResetFlow` | TC-INT-142 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdElevenFlapResetFlow` | TC-INT-144 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdTwelveFlapResetFlow` | TC-INT-146 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdThirteenFlapResetFlow` | TC-INT-148 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFourteenFlapResetFlow` | TC-INT-150 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdFifteenFlapResetFlow` | TC-INT-152 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSixteenFlapResetFlow` | TC-INT-154 |  | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdSeventeenFlapResetFlow` | TC-INT-188 | RV-247 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdEighteenFlapResetFlow` | TC-INT-190 | RV-249 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryHoldoffThresholdNineteenFlapResetFlow` | TC-INT-198 | RV-259 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPreferredRecoveryThresholdCloseoutFlow` | TC-INT-126 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-050 |  | Draft |
| SR-003 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRetransmissionFailoverRepeatedGapUnconfirmedRecoveryProtocolErrorFlow` | TC-INT-051 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h`, `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` | TC-SM-007, TC-SM-008, TC-SM-012, TC-SM-015, TC-SM-017, TC-SM-018 | RV-393 | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-010 | `sil4/include/rsrx_config_validator.h`, `sil4/src/rsrx_config_validator.c` | `rsrx_validate_session_config` | TC-CFG-005, TC-CFG-006, TC-CFG-007, TC-CFG-008 | RV-265, RV-271 | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-011 | `sil4/include/rsrx_protocol_context.h`, `sil4/src/rsrx_protocol_context.c` | `rsrx_protocol_context_init`, `rsrx_protocol_context_record_inbound_message`, `rsrx_protocol_context_build_encode_request`, `rsrx_protocol_context_resolve_inbound_event`, `rsrx_protocol_context_clear_retransmission` | TC-PC-004, TC-PC-020, TC-PC-021, TC-PC-022, TC-PC-025, TC-PC-027, TC-PC-028, TC-PC-029, TC-PC-030, TC-PC-031 | RV-263, RV-266, RV-270, RV-282, RV-385, RV-386, RV-391, RV-419, RV-420, RV-421 | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_init`, `rsrx_channel_manager_update_channel`, `rsrx_channel_manager_select_channel`, `rsrx_channel_manager_get_active_channel`, `rsrx_channel_manager_reset` | TC-CHM-003, TC-CHM-006, TC-CHM-007, TC-CHM-008, TC-CHM-009, TC-CHM-010, TC-CHM-011, TC-CHM-012, TC-CHM-013, TC-CHM-014, TC-CHM-015, TC-CHM-016, TC-CHM-018, TC-CHM-019, TC-CHM-020, TC-CHM-021, TC-CHM-022, TC-CHM-023, TC-CHM-024, TC-CHM-025, TC-CHM-026, TC-CHM-027, TC-CHM-028, TC-CHM-029, TC-CHM-030, TC-CHM-031, TC-CHM-032, TC-CHM-033, TC-CHM-034, TC-CHM-035, TC-CHM-036, TC-CHM-037, TC-CHM-038, TC-CHM-039, TC-CHM-040, TC-CHM-041, TC-CHM-042, TC-CHM-043, TC-CHM-044, TC-CHM-045, TC-CHM-046, TC-CHM-047, TC-CHM-048, TC-CHM-049, TC-CHM-050, TC-CHM-051, TC-CHM-052, TC-CHM-053, TC-CHM-054, TC-CHM-055, TC-CHM-056, TC-CHM-057, TC-CHM-058, TC-CHM-059, TC-CHM-060, TC-CHM-061 | RV-259, RV-260, RV-262, RV-267, RV-271, RV-274, RV-277, RV-283, RV-284, RV-285, RV-286, RV-287, RV-288, RV-289, RV-290, RV-295, RV-368, RV-379, RV-381, RV-387 | Draft |
| SR-003 | HZ-005 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedInitialZeroSequenceProtocolErrorFlow`, `vTestIntegratedDuplicateInboundProtocolErrorFlow`, `vTestIntegratedInvalidConfirmationProtocolErrorFlow`, `vTestIntegratedFailoverInvalidConfirmationProtocolErrorFlow`, `vTestIntegratedRegressingConfirmationProtocolErrorFlow`, `vTestIntegratedFailoverRegressingConfirmationProtocolErrorFlow`, `vTestIntegratedUnconfirmedRecoveryProtocolErrorFlow`, `vTestIntegratedStaleRetransmissionProtocolErrorFlow`, `vTestIntegratedRetransmissionFailoverStaleProtocolErrorFlow` | TC-INT-019, TC-INT-020, TC-INT-021, TC-INT-022, TC-INT-023, TC-INT-028, TC-INT-037, TC-INT-038, TC-INT-039 | RV-030 | Draft |
| SR-003 | HZ-005 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedConnectResponseSequencingRepresentativeFlow` | TC-INT-181 | RV-215 | Draft |
| SR-004 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h`, `sil4/src/rsrx_orchestrator.c` | `rsrx_orchestrator_process_event` | TC-OR-003, TC-OR-004, TC-OR-005 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_platform_port_table_t` | TC-PLAT-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_diagnostics_executor_dispatch`, `rsrx_platform_adapter_build_executor_table` | TC-PA-003 |  | Draft |
| SR-004 | HZ-006 | HLD-001, LLD-007 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_disconnect`, `rsrx_session_process_event` | TC-API-002, TC-API-003, TC-API-006 |  | Draft |
| IF-001 |  | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` |  |  | Draft |
| IF-001 |  | HLD-001, LLD-003 | `sil4/include/rsrx_orchestrator.h` | `rsrx_orchestrator_init`, `rsrx_orchestrator_process_event`, `rsrx_orchestrator_get_state`, `rsrx_orchestrator_reset` | TC-OR-001, TC-OR-004, TC-OR-005, TC-OR-006 | RV-394 | Draft |
| IF-001 |  | HLD-001, LLD-007 | `sil4/include/rsrx_api.h` | `rsrx_session_init`, `rsrx_session_start`, `rsrx_session_connect`, `rsrx_session_disconnect`, `rsrx_session_process_event`, `rsrx_session_process_timer_expiry`, `rsrx_session_get_state`, `rsrx_session_reset` | TC-API-001, TC-API-003, TC-API-004, TC-API-005, TC-API-006, TC-API-007, TC-API-008, TC-API-009, TC-API-012, TC-API-013, TC-API-016, TC-API-017, TC-API-018 | RV-296, RV-297, RV-298, RV-302, RV-392, RV-395, RV-396 | Draft |
| IF-001 |  | HLD-001, LLD-013 | `sil4/include/rsrx_api.h`, `sil4/src/rsrx_api.c` | `rsrx_session_send_application_data` | TC-OUT-001, TC-OUT-002, TC-OUT-003, TC-OUT-005, TC-OUT-008, TC-OUT-009, TC-API-010, TC-API-011 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedPumpReceiveStabilityFlow` | TC-INT-008 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryFlow` | TC-INT-014 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionResetOutboundTelemetryFlow` | TC-INT-205 | RV-303 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSessionRestartAfterResetFlow` | TC-INT-209 | RV-397 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueFifoDispatchFlow` | TC-INT-102 | RV-034 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueMixedClearOrderingFlow` | TC-INT-103 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueMixedClearLongRunFlow` | TC-INT-116 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueLongRunRepresentativeFlow` | TC-INT-185 | RV-224 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestApplicationDataDeferredQueueMixedClearLongRun` | TC-OUT-010 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestBusyRejectThresholdManualInboundResetSources` | TC-OUT-011 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestDeferredQueueTelemetryAccumulationMatrix` | TC-OUT-012 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOverflowBusyAccumulationMatrix` | TC-OUT-013 |  | Draft |
| IF-001 |  | HLD-001, LLD-013 | `sil4/tests/unit/test_rsrx_platform_adapters.c`, `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestApplicationDataSend`, `vTestSupervisorReportExposesBusyRejectTelemetry` | TC-OUT-017, TC-SUP-033 | RV-273 | Draft |
| IF-001 |  | HLD-001, LLD-013 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorReportExposesRuntimeResetTelemetry` | TC-SUP-069 | RV-300 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueLongRunRepresentativeMatrix` | TC-OUT-015 | RV-228 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueFairnessRepresentativeMatrix` | TC-OUT-016 | RV-230 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_platform_adapters.c` | `vTestOutboundQueueBackpressureCloseoutMatrix` | TC-OUT-014 | RV-242 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueOverflowRejectFlow` | TC-INT-015 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdEscalationFlow` | TC-INT-016 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueBackpressureCloseoutFlow` | TC-INT-100 | RV-242 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorQueueReportMatrix` | TC-SUP-059 | RV-226, RV-300 | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdResetFlow` | TC-INT-101 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectThresholdInboundResetFlow` | TC-INT-115 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedBusyRejectAlternatingResetFlow` | TC-INT-117 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedDeferredQueueTelemetryAccumulationFlow` | TC-INT-118 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedQueueOverflowAccumulationFlow` | TC-INT-119 |  | Draft |
| IF-001 |  | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedOverflowBusyAccumulationFlow` | TC-INT-120 |  | Draft |
| IF-002 | HZ-007 | HLD-001 |  |  |  |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_clock_port_t`, `rsrx_timer_port_t`, `rsrx_diagnostics_port_t`, `rsrx_platform_port_table_t` | TC-PLAT-001, TC-PLAT-002 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_platform_adapter_init`, `rsrx_platform_adapter_build_executor_table`, `rsrx_transport_adapter_query_channel`, `rsrx_transport_adapter_reset_runtime_state` | TC-PA-001, TC-PA-006, TC-PA-009, TC-PA-010 | RV-268, RV-299, RV-301 | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-006 | `sil4/include/rsrx_transport.h` | `rsrx_transport_port_t` | TC-TR-001 |  | Draft |
| IF-002 | HZ-007 | HLD-001, LLD-014 | `sil4/include/rsrx_channel_manager.h`, `sil4/src/rsrx_channel_manager.c` | `rsrx_channel_manager_config_t`, `rsrx_channel_selection_result_t`, `rsrx_channel_manager_get_active_channel` | TC-CHM-001, TC-CHM-005, TC-CHM-006, TC-CHM-007, TC-CHM-008, TC-CHM-009, TC-CHM-010, TC-CHM-011, TC-CHM-012, TC-CHM-013, TC-CHM-014, TC-CHM-015, TC-CHM-016, TC-CHM-018, TC-CHM-019, TC-CHM-020, TC-CHM-021, TC-CHM-022, TC-CHM-023, TC-CHM-024, TC-CHM-025, TC-CHM-026, TC-CHM-027, TC-CHM-028, TC-CHM-029, TC-CHM-030, TC-CHM-031, TC-CHM-032, TC-CHM-033, TC-CHM-034, TC-CHM-035, TC-CHM-036, TC-CHM-037, TC-CHM-038, TC-CHM-039, TC-CHM-040, TC-CHM-041, TC-CHM-042, TC-CHM-043, TC-CHM-044, TC-CHM-045, TC-CHM-046, TC-CHM-047, TC-CHM-048, TC-CHM-049, TC-CHM-050, TC-CHM-051, TC-CHM-052, TC-CHM-058, TC-CHM-059, TC-CHM-060 | RV-259, RV-260, RV-262, RV-267, RV-271, RV-274, RV-368, RV-379, RV-381 | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-004 | `sil4/include/rsrx_platform.h` | `rsrx_timer_command_t`, `rsrx_diagnostic_record_t` | TC-PLAT-002, TC-PLAT-003 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_adapter_init`, `rsrx_transport_executor_dispatch`, `rsrx_platform_timer_executor_dispatch`, `rsrx_platform_adapter_init` | TC-PA-002 |  | Draft |
| FR-003 | HZ-003 | HLD-001, LLD-005 | `sil4/include/rsrx_platform_adapters.h`, `sil4/src/rsrx_platform_adapters.c` | `rsrx_transport_adapter_record_inbound_message`, `rsrx_transport_adapter_get_last_inbound_message`, `rsrx_transport_adapter_query_channel`, `rsrx_transport_adapter_reset_runtime_state`, `rsrx_platform_adapter_build_executor_table` | TC-PA-001, TC-PA-004, TC-PA-006, TC-PA-007, TC-PA-008, TC-PA-009, TC-PA-010 | RV-268, RV-299, RV-301 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedChannelRecoveryHoldoffFlow` | TC-INT-012 |  | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedRedundancyFlapSoakFlow` | TC-INT-013 |  | Draft |

## Notes

- 현재 단계에서는 요구사항, hazard, 상위 설계까지만 연결했다.
- 코드, 함수, 테스트, 리뷰 항목은 구현과 검토가 시작되면 채운다.
- 각 행은 삭제하지 말고 상태를 갱신하는 방식으로 유지한다.

| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdSixteenMixedLongRunMatrix` | TC-SUP-058 | RV-184 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdSeventeenMixedLongRunMatrix` | TC-SUP-060 | RV-244 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditTerminalOutcomeThresholdEighteenMixedLongRunMatrix` | TC-SUP-061 | RV-250 | Draft |
| FR-003 | HZ-003 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditActiveLossBypassLongRunMatrix` | TC-SUP-062 | RV-251 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdSixteenMixedLongRunFlow` | TC-INT-179 | RV-184 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdSeventeenMixedLongRunFlow` | TC-INT-186 | RV-244 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdEighteenMixedLongRunFlow` | TC-INT-191 | RV-250 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/integration/test_rsrx_session_supervisor_flow.c` | `vTestIntegratedSwitchAuditTerminalOutcomeThresholdNineteenMixedLongRunFlow` | TC-INT-196 | RV-257 | Draft |
| SR-002 | HZ-002 | HLD-001 | `sil4/tests/unit/test_rsrx_transport_supervisor.c` | `vTestSupervisorSwitchAuditActiveLossBypassLongRunMatrix` | TC-SUP-062 | RV-251 | Draft |
