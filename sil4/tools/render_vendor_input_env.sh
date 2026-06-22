#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_vendor_input_env.sh \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --date <yyyy-mm-dd> \
  --tool-source <tool-name> \
  --commit-id <commit> \
  --ref-name <ref> \
  --run-id <run-id> \
  --job-name <job> \
  --trigger-ref <git-ref> \
  --artifact-name <artifact-name> \
  --tool-version <version> \
  --raw-evidence-type <type> \
  --raw-evidence-location <location> \
  --export-format <format> \
  --capture-timestamp <timestamp> \
  --reviewer-access-path <path> \
  --vendor-rule-id <id> \
  --vendor-rule-family <family> \
  --subset-id <subset> \
  --severity <severity> \
  --file-path <file> \
  --location <location> \
  --initial-decision <decision> \
  --workflow-url <url> \
  --raw-artifact-ref <url-or-ref> \
  --vendor-report-ref <path-or-url> \
  --vendor-review-ref <path-or-url> \
  --vendor-matrix-ref <path-or-url> \
  --tracking-ref <path-or-url> \
  --audit-trail-ref <path-or-url>
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

OUTPUT=""
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

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
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
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

for pair in \
  "--output:$OUTPUT" \
  "--output-dir:$OUTPUT_DIR" \
  "--report-id:$REPORT_ID" \
  "--review-id:$REVIEW_ID" \
  "--date:$DATE_VALUE" \
  "--tool-source:$TOOL_SOURCE" \
  "--commit-id:$COMMIT_ID" \
  "--ref-name:$REF_NAME" \
  "--run-id:$RUN_ID" \
  "--job-name:$JOB_NAME" \
  "--trigger-ref:$TRIGGER_REF" \
  "--artifact-name:$ARTIFACT_NAME" \
  "--tool-version:$TOOL_VERSION" \
  "--raw-evidence-type:$RAW_EVIDENCE_TYPE" \
  "--raw-evidence-location:$RAW_EVIDENCE_LOCATION" \
  "--export-format:$EXPORT_FORMAT" \
  "--capture-timestamp:$CAPTURE_TIMESTAMP" \
  "--reviewer-access-path:$REVIEWER_ACCESS_PATH" \
  "--vendor-rule-id:$VENDOR_RULE_ID" \
  "--vendor-rule-family:$VENDOR_RULE_FAMILY" \
  "--subset-id:$SUBSET_ID" \
  "--severity:$SEVERITY" \
  "--file-path:$FILE_PATH" \
  "--location:$LOCATION" \
  "--initial-decision:$INITIAL_DECISION" \
  "--workflow-url:$WORKFLOW_URL" \
  "--raw-artifact-ref:$RAW_ARTIFACT_REF" \
  "--vendor-report-ref:$VENDOR_REPORT_REF" \
  "--vendor-review-ref:$VENDOR_REVIEW_REF" \
  "--vendor-matrix-ref:$VENDOR_MATRIX_REF" \
  "--tracking-ref:$TRACKING_REF" \
  "--audit-trail-ref:$AUDIT_TRAIL_REF"
do
  require_value "${pair%%:*}" "${pair#*:}"
done

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
OUTPUT_DIR=$OUTPUT_DIR
REPORT_ID=$REPORT_ID
REVIEW_ID=$REVIEW_ID
DATE=$DATE_VALUE
TOOL_SOURCE=$TOOL_SOURCE
COMMIT_ID=$COMMIT_ID
REF_NAME=$REF_NAME
RUN_ID=$RUN_ID
JOB_NAME=$JOB_NAME
TRIGGER_REF=$TRIGGER_REF
ARTIFACT_NAME=$ARTIFACT_NAME
TOOL_VERSION=$TOOL_VERSION
RAW_EVIDENCE_TYPE=$RAW_EVIDENCE_TYPE
RAW_EVIDENCE_LOCATION=$RAW_EVIDENCE_LOCATION
EXPORT_FORMAT=$EXPORT_FORMAT
CAPTURE_TIMESTAMP=$CAPTURE_TIMESTAMP
REVIEWER_ACCESS_PATH=$REVIEWER_ACCESS_PATH
VENDOR_RULE_ID=$VENDOR_RULE_ID
VENDOR_RULE_FAMILY=$VENDOR_RULE_FAMILY
SUBSET_ID=$SUBSET_ID
SEVERITY=$SEVERITY
FILE_PATH=$FILE_PATH
LOCATION=$LOCATION
INITIAL_DECISION=$INITIAL_DECISION
WORKFLOW_URL=$WORKFLOW_URL
RAW_ARTIFACT_REF=$RAW_ARTIFACT_REF
VENDOR_REPORT_REF=$VENDOR_REPORT_REF
VENDOR_REVIEW_REF=$VENDOR_REVIEW_REF
VENDOR_MATRIX_REF=$VENDOR_MATRIX_REF
TRACKING_REF=$TRACKING_REF
AUDIT_TRAIL_REF=$AUDIT_TRAIL_REF
EOF

echo "Rendered vendor input env: $OUTPUT"
