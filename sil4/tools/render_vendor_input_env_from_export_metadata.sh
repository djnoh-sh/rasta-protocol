#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_vendor_input_env_from_export_metadata.sh \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --metadata-env <env-file> \
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

get_env_value() {
  local file="$1"
  local key="$2"
  local line

  if [ ! -f "$file" ]; then
    echo ""
    return
  fi

  line="$(grep -E "^${key}=" "$file" | tail -n 1 || true)"
  if [ -z "$line" ]; then
    echo ""
    return
  fi

  echo "${line#*=}"
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

OUTPUT=""
OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
METADATA_ENV=""
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
    --metadata-env) METADATA_ENV="$2"; shift 2 ;;
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

require_value "--output" "$OUTPUT"
require_value "--output-dir" "$OUTPUT_DIR"
require_value "--report-id" "$REPORT_ID"
require_value "--review-id" "$REVIEW_ID"
require_value "--metadata-env" "$METADATA_ENV"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--raw-artifact-ref" "$RAW_ARTIFACT_REF"
require_value "--vendor-report-ref" "$VENDOR_REPORT_REF"
require_value "--vendor-review-ref" "$VENDOR_REVIEW_REF"
require_value "--vendor-matrix-ref" "$VENDOR_MATRIX_REF"
require_value "--tracking-ref" "$TRACKING_REF"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"

DATE_VALUE="$(get_env_value "$METADATA_ENV" "DATE")"
TOOL_SOURCE="$(get_env_value "$METADATA_ENV" "TOOL_SOURCE")"
COMMIT_ID="$(get_env_value "$METADATA_ENV" "COMMIT_ID")"
REF_NAME="$(get_env_value "$METADATA_ENV" "REF_NAME")"
RUN_ID="$(get_env_value "$METADATA_ENV" "RUN_ID")"
JOB_NAME="$(get_env_value "$METADATA_ENV" "JOB_NAME")"
TRIGGER_REF="$(get_env_value "$METADATA_ENV" "TRIGGER_REF")"
ARTIFACT_NAME="$(get_env_value "$METADATA_ENV" "ARTIFACT_NAME")"
TOOL_VERSION="$(get_env_value "$METADATA_ENV" "TOOL_VERSION")"
RAW_EVIDENCE_TYPE="$(get_env_value "$METADATA_ENV" "RAW_EVIDENCE_TYPE")"
RAW_EVIDENCE_LOCATION="$(get_env_value "$METADATA_ENV" "RAW_EVIDENCE_LOCATION")"
EXPORT_FORMAT="$(get_env_value "$METADATA_ENV" "EXPORT_FORMAT")"
CAPTURE_TIMESTAMP="$(get_env_value "$METADATA_ENV" "CAPTURE_TIMESTAMP")"
REVIEWER_ACCESS_PATH="$(get_env_value "$METADATA_ENV" "REVIEWER_ACCESS_PATH")"
VENDOR_RULE_ID="$(get_env_value "$METADATA_ENV" "VENDOR_RULE_ID")"
VENDOR_RULE_FAMILY="$(get_env_value "$METADATA_ENV" "VENDOR_RULE_FAMILY")"
SUBSET_ID="$(get_env_value "$METADATA_ENV" "SUBSET_ID")"
SEVERITY="$(get_env_value "$METADATA_ENV" "SEVERITY")"
FILE_PATH="$(get_env_value "$METADATA_ENV" "FILE_PATH")"
LOCATION="$(get_env_value "$METADATA_ENV" "LOCATION")"
INITIAL_DECISION="$(get_env_value "$METADATA_ENV" "INITIAL_DECISION")"

require_value "DATE in vendor metadata env" "$DATE_VALUE"
require_value "TOOL_SOURCE in vendor metadata env" "$TOOL_SOURCE"
require_value "COMMIT_ID in vendor metadata env" "$COMMIT_ID"
require_value "REF_NAME in vendor metadata env" "$REF_NAME"
require_value "RUN_ID in vendor metadata env" "$RUN_ID"
require_value "JOB_NAME in vendor metadata env" "$JOB_NAME"
require_value "TRIGGER_REF in vendor metadata env" "$TRIGGER_REF"
require_value "ARTIFACT_NAME in vendor metadata env" "$ARTIFACT_NAME"
require_value "TOOL_VERSION in vendor metadata env" "$TOOL_VERSION"
require_value "RAW_EVIDENCE_TYPE in vendor metadata env" "$RAW_EVIDENCE_TYPE"
require_value "RAW_EVIDENCE_LOCATION in vendor metadata env" "$RAW_EVIDENCE_LOCATION"
require_value "EXPORT_FORMAT in vendor metadata env" "$EXPORT_FORMAT"
require_value "CAPTURE_TIMESTAMP in vendor metadata env" "$CAPTURE_TIMESTAMP"
require_value "REVIEWER_ACCESS_PATH in vendor metadata env" "$REVIEWER_ACCESS_PATH"
require_value "VENDOR_RULE_ID in vendor metadata env" "$VENDOR_RULE_ID"
require_value "VENDOR_RULE_FAMILY in vendor metadata env" "$VENDOR_RULE_FAMILY"
require_value "SUBSET_ID in vendor metadata env" "$SUBSET_ID"
require_value "SEVERITY in vendor metadata env" "$SEVERITY"
require_value "FILE_PATH in vendor metadata env" "$FILE_PATH"
require_value "LOCATION in vendor metadata env" "$LOCATION"
require_value "INITIAL_DECISION in vendor metadata env" "$INITIAL_DECISION"

"$SELF_DIR/render_vendor_input_env.sh" \
  --output "$OUTPUT" \
  --output-dir "$OUTPUT_DIR" \
  --report-id "$REPORT_ID" \
  --review-id "$REVIEW_ID" \
  --date "$DATE_VALUE" \
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
  --audit-trail-ref "$AUDIT_TRAIL_REF"
