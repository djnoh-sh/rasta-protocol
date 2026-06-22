## RV-118 Redundancy Holdoff Threshold Fourteen Review

- Scope
  - `TC-CHM-036`
  - `TC-CHM-037`
  - `TC-INT-149`
  - `TC-INT-150`
- Decision
  - preferred recovery holdoff threshold parity is preserved at `14`
  - flap reset still clears accumulated holdoff before renewed recovery counting resumes
  - higher-threshold closeout family now covers `3/4/5/6/7/8/9/10/11/12/13/14`
- Notes
  - no new switch-audit or bypass semantics were introduced in this batch
  - this step extends only current threshold-family parity inside the existing active-standby policy
