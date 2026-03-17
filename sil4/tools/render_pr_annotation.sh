#!/usr/bin/env bash
set -eu

LOG_DIR="${LOG_DIR:-/tmp/rsrx-ci-logs}"
SUMMARY_ENV="${SUMMARY_ENV:-$LOG_DIR/summary.env}"
ANNOTATION_MD="${ANNOTATION_MD:-$LOG_DIR/pr_annotation.md}"
ANNOTATION_ENV="${ANNOTATION_ENV:-$LOG_DIR/pr_annotation.env}"

if [ ! -f "$SUMMARY_ENV" ]; then
  echo "summary env not found: $SUMMARY_ENV" >&2
  exit 1
fi

# shellcheck disable=SC1090
. "$SUMMARY_ENV"

level="Informational"
reason="clean baseline verification"
review_note="No additional reviewer action required."

if [ "${SEVERITY_CRITICAL_COUNT:-0}" -gt 0 ]; then
  level="Blocker"
  reason="critical severity bucket is non-zero"
  review_note="Merge should be blocked until the critical finding is removed or formally approved."
elif [ "${SEVERITY_HIGH_COUNT:-0}" -gt 0 ]; then
  level="Blocker"
  reason="high severity bucket is non-zero"
  review_note="Merge should be blocked unless the finding is tied to an approved deviation."
elif [ "${SEVERITY_MEDIUM_COUNT:-0}" -gt 0 ]; then
  level="Review Required"
  reason="medium severity bucket requires reviewer attention"
  review_note="Reviewer should confirm mitigation or follow-up action before merge."
fi

priority_subset="none"
if [ "${SUBSET_S1_COUNT:-0}" -gt 0 ]; then
  priority_subset="MISRA-S1"
elif [ "${SUBSET_S2_COUNT:-0}" -gt 0 ]; then
  priority_subset="MISRA-S2"
elif [ "${SUBSET_S3_COUNT:-0}" -gt 0 ]; then
  priority_subset="MISRA-S3"
elif [ "${SUBSET_S4_COUNT:-0}" -gt 0 ]; then
  priority_subset="MISRA-S4"
fi

cat >"$ANNOTATION_MD" <<EOF2
# SIL4 PR Annotation

- Level: **${level}**
- Reason: ${reason}
- Reviewer Note: ${review_note}
- Priority Subset: ${priority_subset}

## Verification Snapshot

| Item | Value |
| --- | --- |
| Tests Passed | ${TEST_COUNT:-0} |
| Compiler Warning Lines | ${COMPILER_WARNING_COUNT:-0} |
| Cppcheck Finding Lines | ${CPPCHECK_FINDING_COUNT:-0} |
| Critical | ${SEVERITY_CRITICAL_COUNT:-0} |
| High | ${SEVERITY_HIGH_COUNT:-0} |
| Medium | ${SEVERITY_MEDIUM_COUNT:-0} |
| Low | ${SEVERITY_LOW_COUNT:-0} |

## Priority Subset Buckets

| Subset | Count |
| --- | --- |
| MISRA-S1 | ${SUBSET_S1_COUNT:-0} |
| MISRA-S2 | ${SUBSET_S2_COUNT:-0} |
| MISRA-S3 | ${SUBSET_S3_COUNT:-0} |
| MISRA-S4 | ${SUBSET_S4_COUNT:-0} |
EOF2

cat >"$ANNOTATION_ENV" <<EOF2
ANNOTATION_LEVEL=$level
ANNOTATION_REASON=$reason
ANNOTATION_REVIEW_NOTE=$review_note
ANNOTATION_PRIORITY_SUBSET=$priority_subset
ANNOTATION_MD=$ANNOTATION_MD
EOF2

echo "Annotation: $ANNOTATION_MD"
