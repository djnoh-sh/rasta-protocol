# RV-187 Redundancy Flap-Bypass Receive Wrapper Review

## Scope
- add a representative integration wrapper for the flap-bypass receive-error branch

## Findings
- the flap-bypass receive branch already has two representative paths:
  - receive-error carryover after bypass
  - receive-error reset after successful primary receive
- those paths were previously only indirect members of the broader flap-bypass closeout family
- the broader stability envelope was therefore less explicit here than for feedback and bypass re-entry branches

## Decision
- add `TC-INT-180` with `vTestIntegratedRedundancyFlapBypassReceiveRepresentativeFlow`
- include that wrapper directly in `vTestIntegratedRedundancyStabilityLongRunRepresentativeFlow`

## Result
- broader redundancy stability tracking now distinguishes:
  - long-run branch
  - non-terminal feedback branch
  - bypass re-entry branch
  - flap-bypass receive branch
