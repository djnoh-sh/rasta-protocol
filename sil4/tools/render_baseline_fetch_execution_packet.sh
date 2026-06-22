#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_baseline_fetch_execution_packet.sh \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --execution-date <yyyy-mm-dd> \
  --commit-id <commit> \
  --ref-name <pr-or-branch> \
  --run-id <workflow-run-id> \
  --trigger-ref <git-ref> \
  --source-type <same-pr|main-branch> \
  --source-run-id <run-id> \
  --workflow-url <url> \
  --artifact-ref <url-or-ref> \
  --resolve-log-ref <text> \
  --download-log-ref <text> \
  --materialize-log-ref <text> \
  --annotate-log-ref <text> \
  [--event-type <event>] \
  [--job-name <job>] \
  [--log-dir <dir>] \
  [--review-result <result>] \
  [--review-summary <summary>] \
  [--tracker-item <EVS-xxx>]
EOF
  exit 1
}

require_value() {
  local name="$1"
  local value="$2"
  if [ -z "$value" ]; then
    echo "missing required argument: $name" >&2
    usage
  fi
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
EXECUTION_DATE=""
COMMIT_ID=""
REF_NAME=""
RUN_ID=""
TRIGGER_REF=""
SOURCE_TYPE=""
SOURCE_RUN_ID=""
WORKFLOW_URL=""
ARTIFACT_REF=""
RESOLVE_LOG_REF=""
DOWNLOAD_LOG_REF=""
MATERIALIZE_LOG_REF=""
ANNOTATE_LOG_REF=""
EVENT_TYPE="pull_request"
JOB_NAME="sil4-verify"
LOG_DIR="/tmp/rsrx-ci-logs"
REVIEW_RESULT="TBD"
REVIEW_SUMMARY="TBD"
TRACKER_ITEM="EVS-001"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --execution-date) EXECUTION_DATE="$2"; shift 2 ;;
    --commit-id) COMMIT_ID="$2"; shift 2 ;;
    --ref-name) REF_NAME="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --trigger-ref) TRIGGER_REF="$2"; shift 2 ;;
    --source-type) SOURCE_TYPE="$2"; shift 2 ;;
    --source-run-id) SOURCE_RUN_ID="$2"; shift 2 ;;
    --workflow-url) WORKFLOW_URL="$2"; shift 2 ;;
    --artifact-ref) ARTIFACT_REF="$2"; shift 2 ;;
    --resolve-log-ref) RESOLVE_LOG_REF="$2"; shift 2 ;;
    --download-log-ref) DOWNLOAD_LOG_REF="$2"; shift 2 ;;
    --materialize-log-ref) MATERIALIZE_LOG_REF="$2"; shift 2 ;;
    --annotate-log-ref) ANNOTATE_LOG_REF="$2"; shift 2 ;;
    --event-type) EVENT_TYPE="$2"; shift 2 ;;
    --job-name) JOB_NAME="$2"; shift 2 ;;
    --log-dir) LOG_DIR="$2"; shift 2 ;;
    --review-result) REVIEW_RESULT="$2"; shift 2 ;;
    --review-summary) REVIEW_SUMMARY="$2"; shift 2 ;;
    --tracker-item) TRACKER_ITEM="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output-dir" "$OUTPUT_DIR"
require_value "--report-id" "$REPORT_ID"
require_value "--review-id" "$REVIEW_ID"
require_value "--execution-date" "$EXECUTION_DATE"
require_value "--commit-id" "$COMMIT_ID"
require_value "--ref-name" "$REF_NAME"
require_value "--run-id" "$RUN_ID"
require_value "--trigger-ref" "$TRIGGER_REF"
require_value "--source-type" "$SOURCE_TYPE"
require_value "--source-run-id" "$SOURCE_RUN_ID"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--artifact-ref" "$ARTIFACT_REF"
require_value "--resolve-log-ref" "$RESOLVE_LOG_REF"
require_value "--download-log-ref" "$DOWNLOAD_LOG_REF"
require_value "--materialize-log-ref" "$MATERIALIZE_LOG_REF"
require_value "--annotate-log-ref" "$ANNOTATE_LOG_REF"

mkdir -p "$OUTPUT_DIR"

REPORT_OUTPUT="$OUTPUT_DIR/baseline_runtime_report.md"
REVIEW_OUTPUT="$OUTPUT_DIR/baseline_runtime_review.md"
TRACKER_OUTPUT="$OUTPUT_DIR/baseline_tracker_row.md"
AUDIT_OUTPUT="$OUTPUT_DIR/baseline_audit_update.md"
REPORT_REF="docs/evidence/reports/$(basename "$REPORT_OUTPUT")"
REVIEW_REF="docs/reviews/$(basename "$REVIEW_OUTPUT")"

"$SELF_DIR/render_baseline_fetch_success_evidence.sh" \
  --output "$REPORT_OUTPUT" \
  --report-id "$REPORT_ID" \
  --execution-date "$EXECUTION_DATE" \
  --commit-id "$COMMIT_ID" \
  --ref-name "$REF_NAME" \
  --run-id "$RUN_ID" \
  --trigger-ref "$TRIGGER_REF" \
  --source-type "$SOURCE_TYPE" \
  --source-run-id "$SOURCE_RUN_ID" \
  --workflow-url "$WORKFLOW_URL" \
  --artifact-ref "$ARTIFACT_REF" \
  --resolve-log-ref "$RESOLVE_LOG_REF" \
  --download-log-ref "$DOWNLOAD_LOG_REF" \
  --materialize-log-ref "$MATERIALIZE_LOG_REF" \
  --annotate-log-ref "$ANNOTATE_LOG_REF" \
  --event-type "$EVENT_TYPE" \
  --job-name "$JOB_NAME" \
  --log-dir "$LOG_DIR"

"$SELF_DIR/render_baseline_fetch_runtime_review.sh" \
  --output "$REVIEW_OUTPUT" \
  --review-id "$REVIEW_ID" \
  --date "$EXECUTION_DATE" \
  --report-ref "$REPORT_REF" \
  --runtime-report-id "$REPORT_ID" \
  --commit-id "$COMMIT_ID" \
  --trigger-ref "$TRIGGER_REF" \
  --run-id "$RUN_ID" \
  --source-type "$SOURCE_TYPE" \
  --source-run-id "$SOURCE_RUN_ID" \
  --materialized-file "$LOG_DIR/baseline_summary.env" \
  --resolve-log-ref "$RESOLVE_LOG_REF" \
  --download-log-ref "$DOWNLOAD_LOG_REF" \
  --materialize-log-ref "$MATERIALIZE_LOG_REF" \
  --annotate-log-ref "$ANNOTATE_LOG_REF" \
  --annotation-mode "delta-aware" \
  --audit-trail-ref "sil4/docs/evidence/audit_trail_closeout.md" \
  --index-ref "sil4/docs/evidence/evidence_index.md" \
  --roadmap-ref "sil4/docs/roadmap_status.md" \
  --result "$REVIEW_RESULT" \
  --summary "$REVIEW_SUMMARY"

"$SELF_DIR/render_evidence_execution_tracker_row.sh" \
  --output "$TRACKER_OUTPUT" \
  --item-id "$TRACKER_ITEM" \
  --status Closed \
  --target-artifact "$REPORT_REF" \
  --execution-note "$EXECUTION_DATE: baseline fetch execution packet scaffolded for run $RUN_ID"

"$SELF_DIR/render_audit_evidence_update.sh" \
  --output "$AUDIT_OUTPUT" \
  --track baseline \
  --date "$EXECUTION_DATE" \
  --report-ref "$REPORT_REF" \
  --review-ref "$REVIEW_REF" \
  --tracker-ref "sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md"

echo "Rendered baseline fetch execution packet in: $OUTPUT_DIR"
