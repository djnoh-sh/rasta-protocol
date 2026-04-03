#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: test_operational_artifact_runner.sh [--work-dir <dir>]
EOF
  exit 1
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
WORK_DIR="/tmp/rsrx-operational-artifact-runner"

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
STARTER_DIR="$WORK_DIR/starter"
STARTER_HANDOFF_SUMMARY_MD="$WORK_DIR/starter_handoff_summary.md"

BASE_LOG_DIR="$WORK_DIR/baseline-logs"
BASE_OUT="$WORK_DIR/baseline-logs-baseline-packet"
BASE_ENV="$BASE_OUT/operational_input.env"
VENDOR_EXPORT_DIR="$WORK_DIR/vendor-export"
VENDOR_OUT="$WORK_DIR/vendor-export-vendor-packet"
VENDOR_ENV="$VENDOR_OUT/operational_input.env"

mkdir -p "$BASE_LOG_DIR" "$VENDOR_EXPORT_DIR"
: > "$BASE_LOG_DIR/summary.env"
: > "$BASE_LOG_DIR/baseline_summary.env"
: > "$BASE_LOG_DIR/pr_annotation.env"
: > "$BASE_LOG_DIR/pr_annotation.md"
: > "$VENDOR_EXPORT_DIR/vendor.xml"

cat >"$BASE_LOG_DIR/baseline_fetch_context.env" <<'EOF2'
EXECUTION_DATE=2026-03-26
COMMIT_ID=abc1234
REF_NAME=refs/pull/123/head
RUN_ID=1001
TRIGGER_REF=refs/pull/123/head
SOURCE_TYPE=same-pr
SOURCE_RUN_ID=999
EOF2

cat >"$VENDOR_EXPORT_DIR/vendor_export_context.env" <<'EOF2'
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
RAW_EVIDENCE_LOCATION=/tmp/rsrx-operational-artifact-runner/vendor-export/vendor.xml
EXPORT_FORMAT=xml
CAPTURE_TIMESTAMP=2026-03-26T10:00:00Z
REVIEWER_ACCESS_PATH=/tmp/rsrx-operational-artifact-runner/vendor-export/vendor.xml
VENDOR_RULE_ID=misra-c2012-2.2
VENDOR_RULE_FAMILY=MISRA
SUBSET_ID=Required
SEVERITY=High
FILE_PATH=sil4/src/example.c
LOCATION=42
INITIAL_DECISION=ReviewRequired
EOF2

sed -i "s|/tmp/rsrx-operational-artifact-runner|$WORK_DIR|g" "$VENDOR_EXPORT_DIR/vendor_export_context.env"
mark_verification_phase_complete "$WORK_DIR" setup

"$SELF_DIR/run_operational_packet_from_artifacts.sh" \
  --track auto \
  --artifact-dir "$BASE_LOG_DIR" \
  --workflow-url https://example.invalid/run/1001 \
  --artifact-ref https://example.invalid/artifacts/1001 \
  --resolve-log-ref resolve-step \
  --download-log-ref download-step \
  --materialize-log-ref materialize-step \
  --annotate-log-ref annotate-step >/dev/null
mark_verification_phase_complete "$WORK_DIR" baseline_execute

"$SELF_DIR/run_operational_packet_from_artifacts.sh" \
  --track auto \
  --artifact-dir "$VENDOR_EXPORT_DIR" \
  --workflow-url https://example.invalid/run/2002 \
  --raw-artifact-ref https://example.invalid/artifacts/2002 \
  --vendor-report-ref sil4/docs/evidence/reports/vendor_runtime_report.md \
  --vendor-review-ref sil4/docs/reviews/vendor_runtime_review.md \
  --vendor-matrix-ref sil4/docs/evidence/vendor_rule_matrix_actual.md \
  --tracking-ref sil4/docs/evidence/misra_deviation_log.md \
  --audit-trail-ref sil4/docs/evidence/audit_trail_closeout.md >/dev/null
mark_verification_phase_complete "$WORK_DIR" vendor_execute

grep -q '^REPORT_ID=EVID-CI-BLRUN-20260326-1001$' "$BASE_ENV"
grep -q '^REVIEW_ID=RV-BLRUN-20260326-1001$' "$BASE_ENV"
grep -q '^REPORT_ID=EVID-CI-VDRUN-20260326-2002$' "$VENDOR_ENV"
grep -q '^REVIEW_ID=RV-VDRUN-20260326-2002$' "$VENDOR_ENV"
[ -f "$BASE_OUT/packet_manifest.md" ]
[ -f "$VENDOR_OUT/packet_manifest.md" ]
[ -f "$BASE_OUT/artifact_runner_summary.env" ]
[ -f "$VENDOR_OUT/artifact_runner_summary.env" ]
[ -f "$BASE_OUT/artifact_runner_receipt.md" ]
[ -f "$VENDOR_OUT/artifact_runner_receipt.md" ]
grep -q "^TRACK=baseline$" "$BASE_OUT/artifact_runner_summary.env"
grep -q "^TRACK=vendor$" "$VENDOR_OUT/artifact_runner_summary.env"
grep -q "^INPUT_ENV=$BASE_ENV$" "$BASE_OUT/artifact_runner_summary.env"
grep -q "^INPUT_ENV=$VENDOR_ENV$" "$VENDOR_OUT/artifact_runner_summary.env"
grep -q "^PACKET_MANIFEST=$BASE_OUT/packet_manifest.md$" "$BASE_OUT/artifact_runner_summary.env"
grep -q "^PACKET_MANIFEST=$VENDOR_OUT/packet_manifest.md$" "$VENDOR_OUT/artifact_runner_summary.env"
grep -q "^# Operational Artifact Runner Receipt$" "$BASE_OUT/artifact_runner_receipt.md"
grep -q "^# Operational Artifact Runner Receipt$" "$VENDOR_OUT/artifact_runner_receipt.md"
grep -q '^- Track: `baseline`$' "$BASE_OUT/artifact_runner_receipt.md"
grep -q '^- Track: `vendor`$' "$VENDOR_OUT/artifact_runner_receipt.md"
bash "$SELF_DIR/validate_operational_artifact_runner_summary.sh" --summary "$BASE_OUT/artifact_runner_summary.env" >/dev/null
bash "$SELF_DIR/validate_operational_artifact_runner_summary.sh" --summary "$VENDOR_OUT/artifact_runner_summary.env" >/dev/null
bash "$SELF_DIR/validate_operational_artifact_runner_receipt.sh" --receipt "$BASE_OUT/artifact_runner_receipt.md" >/dev/null
bash "$SELF_DIR/validate_operational_artifact_runner_receipt.sh" --receipt "$VENDOR_OUT/artifact_runner_receipt.md" >/dev/null
bash "$SELF_DIR/validate_operational_artifact_bundle.sh" --track baseline --output-dir "$BASE_OUT" >/dev/null
bash "$SELF_DIR/validate_operational_artifact_bundle.sh" --track vendor --output-dir "$VENDOR_OUT" >/dev/null

"$SELF_DIR/run_operational_evidence_from_artifact.sh" \
  --artifact-dir "$BASE_LOG_DIR" \
  --work-dir "$STARTER_DIR" \
  --track auto \
  --workflow-url https://example.invalid/run/1001 \
  --artifact-ref https://example.invalid/artifacts/1001 \
  --resolve-log-ref resolve-step \
  --download-log-ref download-step \
  --materialize-log-ref materialize-step \
  --annotate-log-ref annotate-step >/dev/null
[ -f "$STARTER_DIR/readiness/summary.md" ]
[ -f "$STARTER_DIR/readiness_update.md" ]
[ -f "$STARTER_DIR/ready_commands.md" ]
[ -f "$STARTER_DIR/readiness_tracker_rows.md" ]
[ -f "$STARTER_DIR/readiness_audit_note.md" ]
[ -f "$STARTER_DIR/runner.log" ]
"$SELF_DIR/render_operational_evidence_handoff_summary.sh" \
  --output "$STARTER_HANDOFF_SUMMARY_MD" \
  --starter-work-dir "$STARTER_DIR" >/dev/null
[ -f "$STARTER_HANDOFF_SUMMARY_MD" ]
grep -q 'run_operational_packet_from_artifacts.sh --track auto --artifact-dir' "$STARTER_DIR/ready_commands.md"
grep -q '^### Operational Evidence Handoff Summary$' "$STARTER_HANDOFF_SUMMARY_MD"
mark_verification_phase_complete "$WORK_DIR" validate

cat >"$SUMMARY_MD" <<EOF
# Operational Artifact Runner Smoke Summary

| Item | Result |
| --- | --- |
| Setup | Pass |
| Baseline Packet Execution | Pass |
| Vendor Packet Execution | Pass |
| Validation | Pass |
| Verification Ordering Status | Pass |

## Phase Markers

| Phase | Marker |
| --- | --- |
| Setup | \`$(phase_marker_path "$WORK_DIR" setup)\` |
| Baseline Execute | \`$(phase_marker_path "$WORK_DIR" baseline_execute)\` |
| Vendor Execute | \`$(phase_marker_path "$WORK_DIR" vendor_execute)\` |
| Validate | \`$(phase_marker_path "$WORK_DIR" validate)\` |

## Key Outputs

- Baseline env: \`$BASE_ENV\`
- Vendor env: \`$VENDOR_ENV\`
- Baseline output: \`$BASE_OUT\`
- Vendor output: \`$VENDOR_OUT\`
- Starter work dir: \`$STARTER_DIR\`
- Starter handoff summary: \`$STARTER_HANDOFF_SUMMARY_MD\`
EOF

cat >"$SUMMARY_ENV" <<EOF
SUMMARY_MD=$SUMMARY_MD
VERIFICATION_ORDERING_STATUS=Pass
SETUP_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" setup)
BASELINE_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" baseline_execute)
VENDOR_EXECUTE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" vendor_execute)
VALIDATE_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" validate)
BASE_ENV=$BASE_ENV
VENDOR_ENV=$VENDOR_ENV
BASE_OUT=$BASE_OUT
VENDOR_OUT=$VENDOR_OUT
STARTER_DIR=$STARTER_DIR
STARTER_HANDOFF_SUMMARY=$STARTER_HANDOFF_SUMMARY_MD
EOF

echo "Operational artifact runner smoke passed: $WORK_DIR"
echo "Summary: $SUMMARY_MD"
