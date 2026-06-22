## RV-120 Redundancy Holdoff Threshold Sixteen Review

- Scope:
  - `TC-CHM-040`
  - `TC-CHM-041`
  - `TC-INT-153`
  - `TC-INT-154`
- Decision:
  - preferred recovery holdoff threshold parity is preserved at `16`
  - flap reset semantics still reset accumulated holdoff before renewed recovery counting resumes
  - higher-threshold closeout family now covers `3/4/5/6/7/8/9/10/11/12/13/14/15/16`
