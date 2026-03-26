#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: test_operational_input_pipeline.sh [--work-dir <dir>]
EOF
  exit 1
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
WORK_DIR="/tmp/rsrx-operational-input-pipeline"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --work-dir) WORK_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

BASE_LOG_DIR="$WORK_DIR/baseline-logs"
BASE_ENV="$WORK_DIR/baseline.env"
VENDOR_ENV="$WORK_DIR/vendor.env"
BASE_OUT="$WORK_DIR/baseline-packet"
VENDOR_OUT="$WORK_DIR/vendor-packet"
VENDOR_EXPORT_DIR="$WORK_DIR/vendor-export"

mkdir -p "$BASE_LOG_DIR"
mkdir -p "$VENDOR_EXPORT_DIR"
: > "$BASE_LOG_DIR/summary.env"
: > "$BASE_LOG_DIR/baseline_summary.env"
: > "$BASE_LOG_DIR/pr_annotation.env"
: > "$BASE_LOG_DIR/pr_annotation.md"
: > "$VENDOR_EXPORT_DIR/vendor.xml"

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
RAW_EVIDENCE_LOCATION=/tmp/rsrx-operational-input-pipeline/vendor-export/vendor.xml
EXPORT_FORMAT=xml
CAPTURE_TIMESTAMP=2026-03-26T10:00:00Z
REVIEWER_ACCESS_PATH=/tmp/rsrx-operational-input-pipeline/vendor-export/vendor.xml
VENDOR_RULE_ID=misra-c2012-2.2
VENDOR_RULE_FAMILY=MISRA
SUBSET_ID=Required
SEVERITY=High
FILE_PATH=sil4/src/example.c
LOCATION=42
INITIAL_DECISION=ReviewRequired
EOF2

sed -i "s|/tmp/rsrx-operational-input-pipeline|$WORK_DIR|g" "$VENDOR_EXPORT_DIR/vendor_export_context.env"

"$SELF_DIR/render_operational_input_env.sh" \
  --track baseline \
  --output "$BASE_ENV" \
  --output-dir "$BASE_OUT" \
  --report-id EVID-CI-RUN-001 \
  --review-id RV-101 \
  --execution-date 2026-03-26 \
  --commit-id abc1234 \
  --ref-name refs/pull/123/head \
  --run-id 1001 \
  --trigger-ref refs/pull/123/head \
  --source-type same-pr \
  --source-run-id 999 \
  --workflow-url https://example.invalid/run/1001 \
  --artifact-ref https://example.invalid/artifacts/1001 \
  --resolve-log-ref resolve-step \
  --download-log-ref download-step \
  --materialize-log-ref materialize-step \
  --annotate-log-ref annotate-step \
  --log-dir "$BASE_LOG_DIR" >/dev/null

"$SELF_DIR/render_operational_input_env.sh" \
  --track vendor \
  --output "$VENDOR_ENV" \
  --output-dir "$VENDOR_OUT" \
  --report-id EVID-CI-RUN-003 \
  --review-id RV-201 \
  --export-dir "$VENDOR_EXPORT_DIR" \
  --workflow-url https://example.invalid/run/2002 \
  --raw-artifact-ref https://example.invalid/artifacts/2002 \
  --vendor-report-ref sil4/docs/evidence/reports/vendor_runtime_report.md \
  --vendor-review-ref sil4/docs/reviews/vendor_runtime_review.md \
  --vendor-matrix-ref sil4/docs/evidence/vendor_rule_matrix_actual.md \
  --tracking-ref sil4/docs/evidence/misra_deviation_log.md \
  --audit-trail-ref sil4/docs/evidence/audit_trail_closeout.md >/dev/null

"$SELF_DIR/run_operational_packet_from_env.sh" \
  --track baseline \
  --input "$BASE_ENV" \
  --execute >/dev/null

"$SELF_DIR/run_operational_packet_from_env.sh" \
  --track vendor \
  --input "$VENDOR_ENV" \
  --execute >/dev/null

echo "Operational input pipeline smoke passed: $WORK_DIR"
