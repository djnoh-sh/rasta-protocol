# RV-234 Deferred Queue Backlog Depth Four Review

- Scope: `R-004 actual backlog growth from deferred depth 3 to 4`
- Related changes:
  - `D_RSRX_TRANSPORT_ADAPTER_DEFERRED_SEND_CAPACITY` raised to `4U`
  - queue saturation paths updated in adapter, supervisor, and integration tests
  - queue policy wording updated in spec and roadmap

## Checklist

1. current bounded queue policy가 `outstanding 1 + deferred 4` 기준으로 코드와 representative tests에 일관되게 반영됐는지 점검한다.
2. queue overflow/busy accumulation path가 `deferred 4` saturation과 clear ordering 기준으로 다시 닫혔는지 확인한다.
3. `R-004` residual이 current configured depth `4` 이후의 backlog/fairness/runtime-feedback growth로 좁혀졌는지 확인한다.

## Findings

1. deferred send capacity가 named constant 기준으로 `4U`가 됐고, queue saturation/reject guard도 그 깊이에 맞게 동작한다.
2. adapter unit, supervisor unit, session integration queue family가 `deferred 4` saturation과 overflow/busy accumulation path를 기준으로 다시 통과한다.
3. spec과 roadmap wording도 current bounded queue policy를 `outstanding 1 + deferred 4`로 읽도록 갱신됐다.

## Verdict

- Pass. current bounded queue policy는 `outstanding 1 + deferred 4` 기준으로 일관되게 정리됐다.
- Follow-up residual은 current configured depth `4` 이후의 backlog/fairness/runtime-feedback growth로 유지한다.
