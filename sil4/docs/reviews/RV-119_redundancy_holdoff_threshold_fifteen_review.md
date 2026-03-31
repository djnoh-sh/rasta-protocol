## RV-119 Redundancy Holdoff Threshold Fifteen Review

- Scope:
  - `TC-CHM-038`
  - `TC-CHM-039`
  - `TC-INT-151`
  - `TC-INT-152`
- Decision:
  - preferred recovery holdoff threshold parity is preserved at `15`
  - flap reset semantics still reset accumulated holdoff before renewed recovery counting resumes
  - higher-threshold closeout family now covers `3/4/5/6/7/8/9/10/11/12/13/14/15`
