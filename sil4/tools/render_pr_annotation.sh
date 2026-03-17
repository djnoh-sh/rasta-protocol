#!/usr/bin/env bash
set -eu

LOG_DIR="${LOG_DIR:-/tmp/rsrx-ci-logs}"
SUMMARY_ENV="${SUMMARY_ENV:-$LOG_DIR/summary.env}"
BASELINE_SUMMARY_ENV="${BASELINE_SUMMARY_ENV:-$LOG_DIR/baseline_summary.env}"
ANNOTATION_MD="${ANNOTATION_MD:-$LOG_DIR/pr_annotation.md}"
ANNOTATION_ENV="${ANNOTATION_ENV:-$LOG_DIR/pr_annotation.env}"

if [ ! -f "$SUMMARY_ENV" ]; then
  echo "summary env not found: $SUMMARY_ENV" >&2
  exit 1
fi

get_summary_value() {
  local file="$1"
  local key="$2"
  local line

  if [ ! -f "$file" ]; then
    echo 0
    return
  fi

  line="$(grep -E "^${key}=" "$file" | tail -n 1 || true)"
  if [ -z "$line" ]; then
    echo 0
    return
  fi

  echo "${line#*=}"
}

compute_delta() {
  local current_value="$1"
  local baseline_value="$2"
  echo "$((current_value - baseline_value))"
}

# shellcheck disable=SC1090
. "$SUMMARY_ENV"

level="Informational"
reason="clean baseline verification"
review_note="No additional reviewer action required."
baseline_mode="snapshot-only"
delta_available="0"
delta_reason="baseline summary not available"
noise_threshold_note="delta policy inactive"

SEVERITY_CRITICAL_DELTA=0
SEVERITY_HIGH_DELTA=0
SEVERITY_MEDIUM_DELTA=0
SEVERITY_LOW_DELTA=0
SEVERITY_INFO_DELTA=0
SUBSET_S1_DELTA=0
SUBSET_S2_DELTA=0
SUBSET_S3_DELTA=0
SUBSET_S4_DELTA=0
SUBSET_S5_DELTA=0
SUBSET_S6_DELTA=0

if [ -f "$BASELINE_SUMMARY_ENV" ]; then
  baseline_mode="delta-aware"
  delta_available="1"
  delta_reason="baseline summary loaded"
  noise_threshold_note="low/info-only deltas up to 2 stay informational"

  BASELINE_SEVERITY_CRITICAL_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SEVERITY_CRITICAL_COUNT")"
  BASELINE_SEVERITY_HIGH_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SEVERITY_HIGH_COUNT")"
  BASELINE_SEVERITY_MEDIUM_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SEVERITY_MEDIUM_COUNT")"
  BASELINE_SEVERITY_LOW_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SEVERITY_LOW_COUNT")"
  BASELINE_SEVERITY_INFO_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SEVERITY_INFO_COUNT")"
  BASELINE_SUBSET_S1_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S1_COUNT")"
  BASELINE_SUBSET_S2_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S2_COUNT")"
  BASELINE_SUBSET_S3_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S3_COUNT")"
  BASELINE_SUBSET_S4_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S4_COUNT")"
  BASELINE_SUBSET_S5_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S5_COUNT")"
  BASELINE_SUBSET_S6_COUNT="$(get_summary_value "$BASELINE_SUMMARY_ENV" "SUBSET_S6_COUNT")"

  SEVERITY_CRITICAL_DELTA="$(compute_delta "${SEVERITY_CRITICAL_COUNT:-0}" "$BASELINE_SEVERITY_CRITICAL_COUNT")"
  SEVERITY_HIGH_DELTA="$(compute_delta "${SEVERITY_HIGH_COUNT:-0}" "$BASELINE_SEVERITY_HIGH_COUNT")"
  SEVERITY_MEDIUM_DELTA="$(compute_delta "${SEVERITY_MEDIUM_COUNT:-0}" "$BASELINE_SEVERITY_MEDIUM_COUNT")"
  SEVERITY_LOW_DELTA="$(compute_delta "${SEVERITY_LOW_COUNT:-0}" "$BASELINE_SEVERITY_LOW_COUNT")"
  SEVERITY_INFO_DELTA="$(compute_delta "${SEVERITY_INFO_COUNT:-0}" "$BASELINE_SEVERITY_INFO_COUNT")"
  SUBSET_S1_DELTA="$(compute_delta "${SUBSET_S1_COUNT:-0}" "$BASELINE_SUBSET_S1_COUNT")"
  SUBSET_S2_DELTA="$(compute_delta "${SUBSET_S2_COUNT:-0}" "$BASELINE_SUBSET_S2_COUNT")"
  SUBSET_S3_DELTA="$(compute_delta "${SUBSET_S3_COUNT:-0}" "$BASELINE_SUBSET_S3_COUNT")"
  SUBSET_S4_DELTA="$(compute_delta "${SUBSET_S4_COUNT:-0}" "$BASELINE_SUBSET_S4_COUNT")"
  SUBSET_S5_DELTA="$(compute_delta "${SUBSET_S5_COUNT:-0}" "$BASELINE_SUBSET_S5_COUNT")"
  SUBSET_S6_DELTA="$(compute_delta "${SUBSET_S6_COUNT:-0}" "$BASELINE_SUBSET_S6_COUNT")"

  if [ "$SEVERITY_CRITICAL_DELTA" -gt 0 ]; then
    level="Blocker"
    reason="critical severity bucket increased relative to baseline"
    review_note="Merge should be blocked until the new critical finding is removed or formally approved."
  elif [ "$SEVERITY_HIGH_DELTA" -gt 0 ]; then
    level="Blocker"
    reason="high severity bucket increased relative to baseline"
    review_note="Merge should be blocked unless the new high finding is tied to an approved deviation."
  elif [ "$SEVERITY_MEDIUM_DELTA" -gt 0 ]; then
    level="Review Required"
    reason="medium severity bucket increased relative to baseline"
    review_note="Reviewer should confirm mitigation or follow-up action for the new medium finding."
  elif [ "$SEVERITY_LOW_DELTA" -gt 2 ] || [ "$SEVERITY_INFO_DELTA" -gt 2 ]; then
    level="Review Required"
    reason="low/info severity bucket increased beyond noise threshold"
    review_note="Reviewer should confirm that the delta is intentional and low-risk."
  fi
else
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
fi

priority_subset="none"
if [ "$delta_available" = "1" ]; then
  if [ "$SUBSET_S1_DELTA" -gt 0 ]; then
    priority_subset="MISRA-S1"
  elif [ "$SUBSET_S2_DELTA" -gt 0 ]; then
    priority_subset="MISRA-S2"
  elif [ "$SUBSET_S3_DELTA" -gt 0 ]; then
    priority_subset="MISRA-S3"
  elif [ "$SUBSET_S4_DELTA" -gt 0 ]; then
    priority_subset="MISRA-S4"
  fi
else
  if [ "${SUBSET_S1_COUNT:-0}" -gt 0 ]; then
    priority_subset="MISRA-S1"
  elif [ "${SUBSET_S2_COUNT:-0}" -gt 0 ]; then
    priority_subset="MISRA-S2"
  elif [ "${SUBSET_S3_COUNT:-0}" -gt 0 ]; then
    priority_subset="MISRA-S3"
  elif [ "${SUBSET_S4_COUNT:-0}" -gt 0 ]; then
    priority_subset="MISRA-S4"
  fi
fi

cat >"$ANNOTATION_MD" <<EOF2
# SIL4 PR Annotation

- Level: **${level}**
- Reason: ${reason}
- Reviewer Note: ${review_note}
- Priority Subset: ${priority_subset}
- Baseline Mode: ${baseline_mode}
- Delta Status: ${delta_reason}
- Noise Threshold Note: ${noise_threshold_note}

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

## Delta Summary

| Item | Delta |
| --- | --- |
| Critical | ${SEVERITY_CRITICAL_DELTA} |
| High | ${SEVERITY_HIGH_DELTA} |
| Medium | ${SEVERITY_MEDIUM_DELTA} |
| Low | ${SEVERITY_LOW_DELTA} |
| Info | ${SEVERITY_INFO_DELTA} |

## Priority Subset Buckets

| Subset | Count |
| --- | --- |
| MISRA-S1 | ${SUBSET_S1_COUNT:-0} |
| MISRA-S2 | ${SUBSET_S2_COUNT:-0} |
| MISRA-S3 | ${SUBSET_S3_COUNT:-0} |
| MISRA-S4 | ${SUBSET_S4_COUNT:-0} |

## Priority Subset Deltas

| Subset | Delta |
| --- | --- |
| MISRA-S1 | ${SUBSET_S1_DELTA} |
| MISRA-S2 | ${SUBSET_S2_DELTA} |
| MISRA-S3 | ${SUBSET_S3_DELTA} |
| MISRA-S4 | ${SUBSET_S4_DELTA} |
EOF2

cat >"$ANNOTATION_ENV" <<EOF2
ANNOTATION_LEVEL=$level
ANNOTATION_REASON=$reason
ANNOTATION_REVIEW_NOTE=$review_note
ANNOTATION_PRIORITY_SUBSET=$priority_subset
ANNOTATION_BASELINE_MODE=$baseline_mode
ANNOTATION_DELTA_STATUS=$delta_reason
ANNOTATION_SEVERITY_CRITICAL_DELTA=$SEVERITY_CRITICAL_DELTA
ANNOTATION_SEVERITY_HIGH_DELTA=$SEVERITY_HIGH_DELTA
ANNOTATION_SEVERITY_MEDIUM_DELTA=$SEVERITY_MEDIUM_DELTA
ANNOTATION_SEVERITY_LOW_DELTA=$SEVERITY_LOW_DELTA
ANNOTATION_SEVERITY_INFO_DELTA=$SEVERITY_INFO_DELTA
ANNOTATION_SUBSET_S1_DELTA=$SUBSET_S1_DELTA
ANNOTATION_SUBSET_S2_DELTA=$SUBSET_S2_DELTA
ANNOTATION_SUBSET_S3_DELTA=$SUBSET_S3_DELTA
ANNOTATION_SUBSET_S4_DELTA=$SUBSET_S4_DELTA
ANNOTATION_MD=$ANNOTATION_MD
EOF2

echo "Annotation: $ANNOTATION_MD"
