#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: validate_operational_input_env.sh \
  --track <baseline|vendor> \
  --input <env-file>
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

TRACK=""
INPUT=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --input) INPUT="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--track" "$TRACK"
require_value "--input" "$INPUT"

if [ ! -f "$INPUT" ]; then
  echo "input file not found: $INPUT" >&2
  exit 1
fi

# shellcheck disable=SC1090
. "$INPUT"

fail() {
  echo "$1" >&2
  exit 1
}

check_key() {
  local key="$1"
  # shellcheck disable=SC2154
  eval "local value=\"\${$key-}\""
  if [ -z "$value" ]; then
    fail "missing required env var: $key"
  fi
  if [ "$value" = "TBD" ]; then
    fail "placeholder value still present for: $key"
  fi
}

case "$TRACK" in
  baseline)
    for key in OUTPUT_DIR REPORT_ID REVIEW_ID EXECUTION_DATE COMMIT_ID REF_NAME RUN_ID TRIGGER_REF SOURCE_TYPE SOURCE_RUN_ID WORKFLOW_URL ARTIFACT_REF RESOLVE_LOG_REF DOWNLOAD_LOG_REF MATERIALIZE_LOG_REF ANNOTATE_LOG_REF LOG_DIR; do
      check_key "$key"
    done
    ;;
  vendor)
    for key in OUTPUT_DIR REPORT_ID REVIEW_ID DATE TOOL_SOURCE COMMIT_ID REF_NAME RUN_ID JOB_NAME TRIGGER_REF ARTIFACT_NAME TOOL_VERSION RAW_EVIDENCE_TYPE RAW_EVIDENCE_LOCATION EXPORT_FORMAT CAPTURE_TIMESTAMP REVIEWER_ACCESS_PATH VENDOR_RULE_ID VENDOR_RULE_FAMILY SUBSET_ID SEVERITY FILE_PATH LOCATION INITIAL_DECISION WORKFLOW_URL RAW_ARTIFACT_REF VENDOR_REPORT_REF VENDOR_REVIEW_REF VENDOR_MATRIX_REF TRACKING_REF AUDIT_TRAIL_REF; do
      check_key "$key"
    done
    ;;
  *)
    fail "unsupported track: $TRACK"
    ;;
esac

echo "Validated operational input env: $INPUT ($TRACK)"
