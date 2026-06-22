#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_first_actual_vendor_execution_packet.sh \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --date <yyyy-mm-dd> \
  --tool-source <tool-name> \
  --commit-id <commit> \
  --ref-name <pr-or-branch> \
  --run-id <workflow-or-capture-id> \
  --job-name <job-or-capture-name> \
  --trigger-ref <git-ref> \
  --artifact-name <artifact-name> \
  --tool-version <version> \
  --raw-evidence-type <type> \
  --raw-evidence-location <location> \
  --export-format <format> \
  --capture-timestamp <timestamp> \
  --reviewer-access-path <path> \
  --vendor-rule-id <rule-id> \
  --vendor-rule-family <family> \
  --subset-id <subset> \
  --severity <severity> \
  --file-path <file> \
  --location <line-or-range> \
  --initial-decision <decision> \
  --workflow-url <url> \
  --raw-artifact-ref <url-or-ref> \
  --vendor-report-ref <path-or-url> \
  --vendor-review-ref <path-or-url> \
  --vendor-matrix-ref <path-or-url> \
  --tracking-ref <path-or-url> \
  --audit-trail-ref <path-or-url> \
  [--event-type <event>] \
  [--review-result <result>] \
  [--review-summary <summary>] \
  [--matrix-status <status>] \
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
DATE_VALUE=""
TOOL_SOURCE=""
COMMIT_ID=""
REF_NAME=""
RUN_ID=""
JOB_NAME=""
TRIGGER_REF=""
ARTIFACT_NAME=""
TOOL_VERSION=""
RAW_EVIDENCE_TYPE=""
RAW_EVIDENCE_LOCATION=""
EXPORT_FORMAT=""
CAPTURE_TIMESTAMP=""
REVIEWER_ACCESS_PATH=""
VENDOR_RULE_ID=""
VENDOR_RULE_FAMILY=""
SUBSET_ID=""
SEVERITY=""
FILE_PATH=""
LOCATION=""
INITIAL_DECISION=""
WORKFLOW_URL=""
RAW_ARTIFACT_REF=""
VENDOR_REPORT_REF=""
VENDOR_REVIEW_REF=""
VENDOR_MATRIX_REF=""
TRACKING_REF=""
AUDIT_TRAIL_REF=""
EVENT_TYPE="vendor_capture"
REVIEW_RESULT="TBD"
REVIEW_SUMMARY="TBD"
MATRIX_STATUS="Open"
TRACKER_ITEM="EVS-003"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --date) DATE_VALUE="$2"; shift 2 ;;
    --tool-source) TOOL_SOURCE="$2"; shift 2 ;;
    --commit-id) COMMIT_ID="$2"; shift 2 ;;
    --ref-name) REF_NAME="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --job-name) JOB_NAME="$2"; shift 2 ;;
    --trigger-ref) TRIGGER_REF="$2"; shift 2 ;;
    --artifact-name) ARTIFACT_NAME="$2"; shift 2 ;;
    --tool-version) TOOL_VERSION="$2"; shift 2 ;;
    --raw-evidence-type) RAW_EVIDENCE_TYPE="$2"; shift 2 ;;
    --raw-evidence-location) RAW_EVIDENCE_LOCATION="$2"; shift 2 ;;
    --export-format) EXPORT_FORMAT="$2"; shift 2 ;;
    --capture-timestamp) CAPTURE_TIMESTAMP="$2"; shift 2 ;;
    --reviewer-access-path) REVIEWER_ACCESS_PATH="$2"; shift 2 ;;
    --vendor-rule-id) VENDOR_RULE_ID="$2"; shift 2 ;;
    --vendor-rule-family) VENDOR_RULE_FAMILY="$2"; shift 2 ;;
    --subset-id) SUBSET_ID="$2"; shift 2 ;;
    --severity) SEVERITY="$2"; shift 2 ;;
    --file-path) FILE_PATH="$2"; shift 2 ;;
    --location) LOCATION="$2"; shift 2 ;;
    --initial-decision) INITIAL_DECISION="$2"; shift 2 ;;
    --workflow-url) WORKFLOW_URL="$2"; shift 2 ;;
    --raw-artifact-ref) RAW_ARTIFACT_REF="$2"; shift 2 ;;
    --vendor-report-ref) VENDOR_REPORT_REF="$2"; shift 2 ;;
    --vendor-review-ref) VENDOR_REVIEW_REF="$2"; shift 2 ;;
    --vendor-matrix-ref) VENDOR_MATRIX_REF="$2"; shift 2 ;;
    --tracking-ref) TRACKING_REF="$2"; shift 2 ;;
    --audit-trail-ref) AUDIT_TRAIL_REF="$2"; shift 2 ;;
    --event-type) EVENT_TYPE="$2"; shift 2 ;;
    --review-result) REVIEW_RESULT="$2"; shift 2 ;;
    --review-summary) REVIEW_SUMMARY="$2"; shift 2 ;;
    --matrix-status) MATRIX_STATUS="$2"; shift 2 ;;
    --tracker-item) TRACKER_ITEM="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output-dir" "$OUTPUT_DIR"
require_value "--report-id" "$REPORT_ID"
require_value "--review-id" "$REVIEW_ID"
require_value "--date" "$DATE_VALUE"
require_value "--tool-source" "$TOOL_SOURCE"
require_value "--commit-id" "$COMMIT_ID"
require_value "--ref-name" "$REF_NAME"
require_value "--run-id" "$RUN_ID"
require_value "--job-name" "$JOB_NAME"
require_value "--trigger-ref" "$TRIGGER_REF"
require_value "--artifact-name" "$ARTIFACT_NAME"
require_value "--tool-version" "$TOOL_VERSION"
require_value "--raw-evidence-type" "$RAW_EVIDENCE_TYPE"
require_value "--raw-evidence-location" "$RAW_EVIDENCE_LOCATION"
require_value "--export-format" "$EXPORT_FORMAT"
require_value "--capture-timestamp" "$CAPTURE_TIMESTAMP"
require_value "--reviewer-access-path" "$REVIEWER_ACCESS_PATH"
require_value "--vendor-rule-id" "$VENDOR_RULE_ID"
require_value "--vendor-rule-family" "$VENDOR_RULE_FAMILY"
require_value "--subset-id" "$SUBSET_ID"
require_value "--severity" "$SEVERITY"
require_value "--file-path" "$FILE_PATH"
require_value "--location" "$LOCATION"
require_value "--initial-decision" "$INITIAL_DECISION"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--raw-artifact-ref" "$RAW_ARTIFACT_REF"
require_value "--vendor-report-ref" "$VENDOR_REPORT_REF"
require_value "--vendor-review-ref" "$VENDOR_REVIEW_REF"
require_value "--vendor-matrix-ref" "$VENDOR_MATRIX_REF"
require_value "--tracking-ref" "$TRACKING_REF"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"

mkdir -p "$OUTPUT_DIR"

REPORT_OUTPUT="$OUTPUT_DIR/vendor_runtime_report.md"
REVIEW_OUTPUT="$OUTPUT_DIR/vendor_runtime_review.md"
MATRIX_OUTPUT="$OUTPUT_DIR/vendor_matrix_row.md"
TRACKER_OUTPUT="$OUTPUT_DIR/vendor_tracker_row.md"
AUDIT_OUTPUT="$OUTPUT_DIR/vendor_audit_update.md"

"$SELF_DIR/render_first_actual_vendor_evidence.sh" \
  --output "$REPORT_OUTPUT" \
  --report-id "$REPORT_ID" \
  --execution-date "$DATE_VALUE" \
  --tool-source "$TOOL_SOURCE" \
  --commit-id "$COMMIT_ID" \
  --ref-name "$REF_NAME" \
  --run-id "$RUN_ID" \
  --job-name "$JOB_NAME" \
  --trigger-ref "$TRIGGER_REF" \
  --artifact-name "$ARTIFACT_NAME" \
  --tool-version "$TOOL_VERSION" \
  --raw-evidence-type "$RAW_EVIDENCE_TYPE" \
  --raw-evidence-location "$RAW_EVIDENCE_LOCATION" \
  --export-format "$EXPORT_FORMAT" \
  --capture-timestamp "$CAPTURE_TIMESTAMP" \
  --reviewer-access-path "$REVIEWER_ACCESS_PATH" \
  --vendor-rule-id "$VENDOR_RULE_ID" \
  --vendor-rule-family "$VENDOR_RULE_FAMILY" \
  --subset-id "$SUBSET_ID" \
  --severity "$SEVERITY" \
  --file-path "$FILE_PATH" \
  --location "$LOCATION" \
  --initial-decision "$INITIAL_DECISION" \
  --workflow-url "$WORKFLOW_URL" \
  --raw-artifact-ref "$RAW_ARTIFACT_REF" \
  --vendor-report-ref "$VENDOR_REPORT_REF" \
  --vendor-review-ref "$VENDOR_REVIEW_REF" \
  --vendor-matrix-ref "$VENDOR_MATRIX_REF" \
  --tracking-ref "$TRACKING_REF" \
  --audit-trail-ref "$AUDIT_TRAIL_REF" \
  --event-type "$EVENT_TYPE"

"$SELF_DIR/render_first_actual_vendor_runtime_review.sh" \
  --output "$REVIEW_OUTPUT" \
  --review-id "$REVIEW_ID" \
  --date "$DATE_VALUE" \
  --report-ref "$VENDOR_REPORT_REF" \
  --runtime-report-id "$REPORT_ID" \
  --matrix-ref "$VENDOR_MATRIX_REF" \
  --tracking-ref "$TRACKING_REF" \
  --raw-evidence-ref "$RAW_ARTIFACT_REF" \
  --rule-id "$VENDOR_RULE_ID" \
  --subset-id "$SUBSET_ID" \
  --severity "$SEVERITY" \
  --file-path "$FILE_PATH" \
  --location "$LOCATION" \
  --decision-ref "$INITIAL_DECISION" \
  --audit-trail-ref "$AUDIT_TRAIL_REF" \
  --index-ref "sil4/docs/evidence/evidence_index.md" \
  --roadmap-ref "sil4/docs/roadmap_status.md" \
  --result "$REVIEW_RESULT" \
  --summary "$REVIEW_SUMMARY"

"$SELF_DIR/render_vendor_matrix_actual_row.sh" \
  --output "$MATRIX_OUTPUT" \
  --tool-source "$TOOL_SOURCE" \
  --run-id "$RUN_ID" \
  --rule-id "$VENDOR_RULE_ID" \
  --rule-family "$VENDOR_RULE_FAMILY" \
  --subset-id "$SUBSET_ID" \
  --severity "$SEVERITY" \
  --file-path "$FILE_PATH" \
  --location "$LOCATION" \
  --initial-decision "$INITIAL_DECISION" \
  --report-link "$VENDOR_REPORT_REF" \
  --review-link "$VENDOR_REVIEW_REF" \
  --tracking-link "$TRACKING_REF" \
  --status "$MATRIX_STATUS"

"$SELF_DIR/render_evidence_execution_tracker_row.sh" \
  --output "$TRACKER_OUTPUT" \
  --item-id "$TRACKER_ITEM" \
  --status Closed \
  --target-artifact "$VENDOR_REPORT_REF" \
  --execution-note "$DATE_VALUE: vendor execution packet scaffolded for $VENDOR_RULE_ID"

"$SELF_DIR/render_audit_evidence_update.sh" \
  --output "$AUDIT_OUTPUT" \
  --track vendor \
  --date "$DATE_VALUE" \
  --report-ref "$VENDOR_REPORT_REF" \
  --review-ref "$VENDOR_REVIEW_REF" \
  --tracker-ref "sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md" \
  --matrix-ref "$VENDOR_MATRIX_REF" \
  --tracking-ref "$TRACKING_REF"

echo "Rendered first actual vendor execution packet in: $OUTPUT_DIR"
