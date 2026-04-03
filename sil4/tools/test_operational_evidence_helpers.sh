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
READINESS_DIR="$WORK_DIR/readiness"
READINESS_UPDATE_MD="$WORK_DIR/readiness_update.md"
READY_COMMANDS_MD="$WORK_DIR/ready_commands.md"
READINESS_TRACKER_ROWS_MD="$WORK_DIR/readiness_tracker_rows.md"

BASE_LOG_DIR="$WORK_DIR/baseline-logs"
BASE_ENV="$WORK_DIR/baseline.env"
VENDOR_ENV="$WORK_DIR/vendor.env"
BASE_OUT="$WORK_DIR/baseline-packet"
VENDOR_OUT="$WORK_DIR/vendor-packet"
BASE_ARTIFACT_DIR="$WORK_DIR/baseline-artifacts"
VENDOR_ARTIFACT_DIR="$WORK_DIR/vendor-artifacts"

mkdir -p "$BASE_LOG_DIR" "$BASE_ARTIFACT_DIR" "$VENDOR_ARTIFACT_DIR"
: > "$BASE_LOG_DIR/summary.env"
: > "$BASE_LOG_DIR/baseline_summary.env"
: > "$BASE_LOG_DIR/pr_annotation.env"
: > "$BASE_LOG_DIR/pr_annotation.md"
: > "$WORK_DIR/vendor.xml"
: > "$VENDOR_ARTIFACT_DIR/vendor.xml"

cat >"$BASE_ARTIFACT_DIR/baseline_fetch_context.env" <<EOF
EXECUTION_DATE=2026-03-26
RUN_ID=1001
EOF

cat >"$VENDOR_ARTIFACT_DIR/vendor_export_context.env" <<EOF
DATE=2026-03-26
RUN_ID=2002
RAW_EVIDENCE_LOCATION=$VENDOR_ARTIFACT_DIR/vendor.xml
REVIEWER_ACCESS_PATH=$VENDOR_ARTIFACT_DIR/vendor.xml
EOF

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

"$SELF_DIR/check_operational_evidence_readiness.sh" \
  --work-dir "$READINESS_DIR" \
  --baseline-artifact-dir "$BASE_ARTIFACT_DIR" \
  --vendor-artifact-dir "$VENDOR_ARTIFACT_DIR" \
  --require-ready >/dev/null
grep -q '^OVERALL_READINESS_STATUS=Ready$' "$READINESS_DIR/summary.env"
grep -q '^BASELINE_ARTIFACT_STATUS=Available$' "$READINESS_DIR/summary.env"
grep -q '^VENDOR_ARTIFACT_STATUS=Available$' "$READINESS_DIR/summary.env"
"$SELF_DIR/render_operational_evidence_readiness_update.sh" \
  --output "$READINESS_UPDATE_MD" \
  --summary-env "$READINESS_DIR/summary.env" \
  --tracker-ref "sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md" \
  --handoff-ref "sil4/docs/evidence/first_operational_evidence_handoff_sheet.md" \
  --execution-packet-ref "sil4/docs/evidence/first_operational_evidence_execution_packet.md" >/dev/null
"$SELF_DIR/render_operational_evidence_ready_commands.sh" \
  --output "$READY_COMMANDS_MD" \
  --summary-env "$READINESS_DIR/summary.env" >/dev/null
"$SELF_DIR/render_operational_evidence_readiness_tracker_rows.sh" \
  --output "$READINESS_TRACKER_ROWS_MD" \
  --summary-env "$READINESS_DIR/summary.env" >/dev/null
grep -q '^### Operational Evidence Readiness Update$' "$READINESS_UPDATE_MD"
grep -q 'overall readiness: `Ready`' "$READINESS_UPDATE_MD"
grep -q 'execution tracker target:' "$READINESS_UPDATE_MD"
grep -q '^### Operational Evidence Ready Commands$' "$READY_COMMANDS_MD"
grep -q 'run_operational_packet_from_artifacts.sh --track auto --artifact-dir' "$READY_COMMANDS_MD"
grep -q '| EVS-001 | `In Progress` | baseline fetch success runtime evidence |' "$READINESS_TRACKER_ROWS_MD"
grep -q '| EVS-003 | `In Progress` | vendor raw evidence reference |' "$READINESS_TRACKER_ROWS_MD"
mark_verification_phase_complete "$WORK_DIR" readiness_check

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
| Readiness Check | Pass |
| Baseline Packet Execution | Pass |
| Vendor Packet Execution | Pass |
| Verification Ordering Status | Pass |

## Phase Markers

| Phase | Marker |
| --- | --- |
| Setup | \`$(phase_marker_path "$WORK_DIR" setup)\` |
| Readiness Check | \`$(phase_marker_path "$WORK_DIR" readiness_check)\` |
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
READINESS_CHECK_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" readiness_check)
BASELINE_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" baseline_execute)
VENDOR_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" vendor_execute)
READINESS_SUMMARY=$READINESS_DIR/summary.md
READINESS_UPDATE=$READINESS_UPDATE_MD
READY_COMMANDS=$READY_COMMANDS_MD
READINESS_TRACKER_ROWS=$READINESS_TRACKER_ROWS_MD
BASE_ENV=$BASE_ENV
VENDOR_ENV=$VENDOR_ENV
BASE_OUT=$BASE_OUT
VENDOR_OUT=$VENDOR_OUT
EOF

echo "Operational evidence helper smoke passed: $WORK_DIR"
echo "Summary: $SUMMARY_MD"
