#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_baseline_fetch_input_env.sh \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --execution-date <yyyy-mm-dd> \
  --commit-id <commit> \
  --ref-name <ref> \
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
  --log-dir <dir>
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
LOG_DIR=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
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
    --log-dir) LOG_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
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
require_value "--log-dir" "$LOG_DIR"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
OUTPUT_DIR=$OUTPUT_DIR
REPORT_ID=$REPORT_ID
REVIEW_ID=$REVIEW_ID
EXECUTION_DATE=$EXECUTION_DATE
COMMIT_ID=$COMMIT_ID
REF_NAME=$REF_NAME
RUN_ID=$RUN_ID
TRIGGER_REF=$TRIGGER_REF
SOURCE_TYPE=$SOURCE_TYPE
SOURCE_RUN_ID=$SOURCE_RUN_ID
WORKFLOW_URL=$WORKFLOW_URL
ARTIFACT_REF=$ARTIFACT_REF
RESOLVE_LOG_REF=$RESOLVE_LOG_REF
DOWNLOAD_LOG_REF=$DOWNLOAD_LOG_REF
MATERIALIZE_LOG_REF=$MATERIALIZE_LOG_REF
ANNOTATE_LOG_REF=$ANNOTATE_LOG_REF
LOG_DIR=$LOG_DIR
EOF

echo "Rendered baseline fetch input env: $OUTPUT"
