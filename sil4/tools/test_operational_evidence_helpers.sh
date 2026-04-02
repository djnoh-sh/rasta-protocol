#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: test_operational_evidence_helpers.sh [--work-dir <dir>]
EOF
  exit 1
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
WORK_DIR="/tmp/rsrx-operational-smoke"

. "$SELF_DIR/verification_sequence_common.sh"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --work-dir) WORK_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

SUMMARY_MD="$WORK_DIR/summary.md"
SUMMARY_ENV="$WORK_DIR/summary.env"

BASE_LOG_DIR="$WORK_DIR/baseline-logs"
BASE_ENV="$WORK_DIR/baseline.env"
VENDOR_ENV="$WORK_DIR/vendor.env"
BASE_OUT="$WORK_DIR/baseline-packet"
VENDOR_OUT="$WORK_DIR/vendor-packet"

mkdir -p "$BASE_LOG_DIR"
: > "$BASE_LOG_DIR/summary.env"
: > "$BASE_LOG_DIR/baseline_summary.env"
: > "$BASE_LOG_DIR/pr_annotation.env"
: > "$BASE_LOG_DIR/pr_annotation.md"
: > "$WORK_DIR/vendor.xml"

cat >"$BASE_ENV" <<EOF
OUTPUT_DIR=$BASE_OUT
REPORT_ID=EVID-CI-RUN-001
REVIEW_ID=RV-101
EXECUTION_DATE=2026-03-26
COMMIT_ID=abc1234
REF_NAME=refs/pull/123/head
RUN_ID=1001
TRIGGER_REF=refs/pull/123/head
SOURCE_TYPE=same-pr
SOURCE_RUN_ID=999
WORKFLOW_URL=https://example.invalid/run/1001
ARTIFACT_REF=https://example.invalid/artifacts/1001
RESOLVE_LOG_REF=resolve-step
DOWNLOAD_LOG_REF=download-step
MATERIALIZE_LOG_REF=materialize-step
ANNOTATE_LOG_REF=annotate-step
LOG_DIR=$BASE_LOG_DIR
EOF

cat >"$VENDOR_ENV" <<EOF
OUTPUT_DIR=$VENDOR_OUT
REPORT_ID=EVID-CI-RUN-003
REVIEW_ID=RV-201
DATE=2026-03-26
TOOL_SOURCE=cppcheck
COMMIT_ID=abc1234
REF_NAME=refs/heads/main
RUN_ID=2002
JOB_NAME=vendor-capture
TRIGGER_REF=refs/heads/main
ARTIFACT_NAME=vendor-export
TOOL_VERSION=2.14
RAW_EVIDENCE_TYPE=xml
RAW_EVIDENCE_LOCATION=$WORK_DIR/vendor.xml
EXPORT_FORMAT=xml
CAPTURE_TIMESTAMP=2026-03-26T10:00:00Z
REVIEWER_ACCESS_PATH=$WORK_DIR/vendor.xml
VENDOR_RULE_ID=misra-c2012-2.2
VENDOR_RULE_FAMILY=MISRA
SUBSET_ID=Required
SEVERITY=High
FILE_PATH=sil4/src/example.c
LOCATION=42
INITIAL_DECISION=ReviewRequired
WORKFLOW_URL=https://example.invalid/run/2002
RAW_ARTIFACT_REF=https://example.invalid/artifacts/2002
VENDOR_REPORT_REF=sil4/docs/evidence/reports/vendor_runtime_report.md
VENDOR_REVIEW_REF=sil4/docs/reviews/vendor_runtime_review.md
VENDOR_MATRIX_REF=sil4/docs/evidence/vendor_rule_matrix_actual.md
TRACKING_REF=sil4/docs/evidence/misra_deviation_log.md
AUDIT_TRAIL_REF=sil4/docs/evidence/audit_trail_closeout.md
EOF
mark_verification_phase_complete "$WORK_DIR" setup

"$SELF_DIR/run_operational_packet_from_env.sh" \
  --track baseline \
  --input "$BASE_ENV" \
  --execute >/dev/null
mark_verification_phase_complete "$WORK_DIR" baseline_execute

"$SELF_DIR/run_operational_packet_from_env.sh" \
  --track vendor \
  --input "$VENDOR_ENV" \
  --execute >/dev/null
mark_verification_phase_complete "$WORK_DIR" vendor_execute

cat >"$SUMMARY_MD" <<EOF
# Operational Evidence Helper Smoke Summary

| Item | Result |
| --- | --- |
| Setup | Pass |
| Baseline Packet Execution | Pass |
| Vendor Packet Execution | Pass |
| Verification Ordering Status | Pass |

## Phase Markers

| Phase | Marker |
| --- | --- |
| Setup | \`$(phase_marker_path "$WORK_DIR" setup)\` |
| Baseline Execute | \`$(phase_marker_path "$WORK_DIR" baseline_execute)\` |
| Vendor Execute | \`$(phase_marker_path "$WORK_DIR" vendor_execute)\` |

## Key Outputs

- Baseline env: \`$BASE_ENV\`
- Vendor env: \`$VENDOR_ENV\`
- Baseline output: \`$BASE_OUT\`
- Vendor output: \`$VENDOR_OUT\`
EOF

cat >"$SUMMARY_ENV" <<EOF
SUMMARY_MD=$SUMMARY_MD
VERIFICATION_ORDERING_STATUS=Pass
SETUP_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" setup)
BASELINE_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" baseline_execute)
VENDOR_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" vendor_execute)
BASE_ENV=$BASE_ENV
VENDOR_ENV=$VENDOR_ENV
BASE_OUT=$BASE_OUT
VENDOR_OUT=$VENDOR_OUT
EOF

echo "Operational evidence helper smoke passed: $WORK_DIR"
echo "Summary: $SUMMARY_MD"
