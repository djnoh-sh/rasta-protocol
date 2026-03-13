# Initial Traceability Matrix

| Req ID | Safety ID | Design ID | Source File | Function | Test ID | Review Record | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FR-001 |  | HLD-001 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init` |  |  | Draft |
| FR-002 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-001, TC-SM-002, TC-SM-009 |  | Draft |
| FR-003 | HZ-003 | HLD-001 |  |  |  |  | Draft |
| FR-004 | HZ-003 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-004, TC-SM-005, TC-SM-011 |  | Draft |
| FR-005 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event`, `rsrx_state_machine_reset` | TC-SM-006, TC-SM-010 |  | Draft |
| FR-006 | HZ-004 | HLD-001 |  |  |  |  | Draft |
| FR-007 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| SR-001 | HZ-001 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-002 |  | Draft |
| SR-002 | HZ-002 | HLD-001, LLD-002 | `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_handle_event` | TC-SM-003 |  | Draft |
| SR-003 | HZ-005 | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h`, `sil4/src/rsrx_state_machine.c` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` | TC-SM-007, TC-SM-008, TC-SM-012 |  | Draft |
| SR-004 | HZ-006 | HLD-001 |  |  |  |  | Draft |
| IF-001 |  | HLD-001, LLD-002 | `sil4/include/rsrx_state_machine.h` | `rsrx_state_machine_init`, `rsrx_state_machine_handle_event`, `rsrx_state_machine_get_state`, `rsrx_state_machine_reset` |  |  | Draft |
| IF-002 | HZ-007 | HLD-001 |  |  |  |  | Draft |

## Notes

- 현재 단계에서는 요구사항, hazard, 상위 설계까지만 연결했다.
- 코드, 함수, 테스트, 리뷰 항목은 구현과 검토가 시작되면 채운다.
- 각 행은 삭제하지 말고 상태를 갱신하는 방식으로 유지한다.
