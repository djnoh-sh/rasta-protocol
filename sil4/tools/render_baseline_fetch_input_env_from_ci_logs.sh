#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_baseline_fetch_input_env_from_ci_logs.sh \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --log-dir <dir> \
  --workflow-url <url> \
  --artifact-ref <url-or-ref> \
  --resolve-log-ref <text> \
  --download-log-ref <text> \
  --materialize-log-ref <text> \
  --annotate-log-ref <text>
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
LOG_DIR=""
WORKFLOW_URL=""
ARTIFACT_REF=""
RESOLVE_LOG_REF=""
DOWNLOAD_LOG_REF=""
MATERIALIZE_LOG_REF=""
ANNOTATE_LOG_REF=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --log-dir) LOG_DIR="$2"; shift 2 ;;
    --workflow-url) WORKFLOW_URL="$2"; shift 2 ;;
    --artifact-ref) ARTIFACT_REF="$2"; shift 2 ;;
    --resolve-log-ref) RESOLVE_LOG_REF="$2"; shift 2 ;;
    --download-log-ref) DOWNLOAD_LOG_REF="$2"; shift 2 ;;
    --materialize-log-ref) MATERIALIZE_LOG_REF="$2"; shift 2 ;;
    --annotate-log-ref) ANNOTATE_LOG_REF="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--output-dir" "$OUTPUT_DIR"
require_value "--report-id" "$REPORT_ID"
require_value "--review-id" "$REVIEW_ID"
require_value "--log-dir" "$LOG_DIR"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--artifact-ref" "$ARTIFACT_REF"
require_value "--resolve-log-ref" "$RESOLVE_LOG_REF"
require_value "--download-log-ref" "$DOWNLOAD_LOG_REF"
require_value "--materialize-log-ref" "$MATERIALIZE_LOG_REF"
require_value "--annotate-log-ref" "$ANNOTATE_LOG_REF"

CONTEXT_ENV="$LOG_DIR/baseline_fetch_context.env"

EXECUTION_DATE="$(get_env_value "$CONTEXT_ENV" "EXECUTION_DATE")"
COMMIT_ID="$(get_env_value "$CONTEXT_ENV" "COMMIT_ID")"
REF_NAME="$(get_env_value "$CONTEXT_ENV" "REF_NAME")"
RUN_ID="$(get_env_value "$CONTEXT_ENV" "RUN_ID")"
TRIGGER_REF="$(get_env_value "$CONTEXT_ENV" "TRIGGER_REF")"
SOURCE_TYPE="$(get_env_value "$CONTEXT_ENV" "SOURCE_TYPE")"
SOURCE_RUN_ID="$(get_env_value "$CONTEXT_ENV" "SOURCE_RUN_ID")"

require_value "EXECUTION_DATE in baseline_fetch_context.env" "$EXECUTION_DATE"
require_value "COMMIT_ID in baseline_fetch_context.env" "$COMMIT_ID"
require_value "REF_NAME in baseline_fetch_context.env" "$REF_NAME"
require_value "RUN_ID in baseline_fetch_context.env" "$RUN_ID"
require_value "TRIGGER_REF in baseline_fetch_context.env" "$TRIGGER_REF"
require_value "SOURCE_TYPE in baseline_fetch_context.env" "$SOURCE_TYPE"
require_value "SOURCE_RUN_ID in baseline_fetch_context.env" "$SOURCE_RUN_ID"

"$SELF_DIR/render_baseline_fetch_input_env.sh" \
  --output "$OUTPUT" \
  --output-dir "$OUTPUT_DIR" \
  --report-id "$REPORT_ID" \
  --review-id "$REVIEW_ID" \
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
  --log-dir "$LOG_DIR"
