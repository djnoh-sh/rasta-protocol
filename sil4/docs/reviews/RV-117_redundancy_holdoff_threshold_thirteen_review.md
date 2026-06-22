## RV-117 Redundancy Holdoff Threshold Thirteen Review

- Scope
  - `TC-CHM-034`
  - `TC-CHM-035`
  - `TC-INT-147`
  - `TC-INT-148`
- Decision
  - preferred recovery holdoff threshold parity is preserved at `13`
  - flap reset still clears accumulated holdoff before renewed recovery counting resumes
  - higher-threshold closeout family now covers `3/4/5/6/7/8/9/10/11/12/13`
- Notes
  - no new switch-audit or bypass semantics were introduced in this batch
  - this step extends only current threshold-family parity inside the existing active-standby policy
