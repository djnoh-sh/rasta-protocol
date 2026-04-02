# RV-169 Cppcheck Suppress Dedup Review

## Decision

인접한 동일 `cppcheck-suppress redundantAssignment` 주석은 하나만 유지한다.

## Rationale

- 같은 suppress 주석이 연속으로 여러 줄 누적되면 읽기 비용만 증가하고, 실제 suppress 의도를 더 명확하게 만들지 않는다.
- 특히 테스트 코드에서는 반복 패치 과정에서 동일 suppress가 인접 중복으로 남기 쉽다.
- suppress는 "왜 여기 필요한가"가 보여야 하므로, 인접 중복은 제거하고 필요한 위치에 단일 주석만 남기는 편이 더 정확하다.

## Consequence

- 이번 정리는 [test_rsrx_transport_supervisor.c](/home/djnoh/repos/rasta-protocol/sil4/tests/unit/test_rsrx_transport_supervisor.c)와 [test_rsrx_session_supervisor_flow.c](/home/djnoh/repos/rasta-protocol/sil4/tests/integration/test_rsrx_session_supervisor_flow.c)의 adjacent duplicate suppress만 제거한다.
- 앞으로는 suppress를 추가하기 전에 바로 위/아래 인접 줄의 동일 suppress 존재 여부를 먼저 확인한다.
