#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_packet_invocation.sh \
  --track <baseline|vendor> \
  --input <env-file> \
  [--output <command-sh>]
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
OUTPUT=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --input) INPUT="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
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

require_env() {
  local name="$1"
  # shellcheck disable=SC2154
  eval "local value=\"\${$name-}\""
  if [ -z "$value" ]; then
    echo "missing required env var in input: $name" >&2
    exit 1
  fi
}

emit() {
  if [ -n "$OUTPUT" ]; then
    mkdir -p "$(dirname "$OUTPUT")"
    cat >"$OUTPUT"
  else
    cat
  fi
}

case "$TRACK" in
  baseline)
    for key in OUTPUT_DIR REPORT_ID REVIEW_ID EXECUTION_DATE COMMIT_ID REF_NAME RUN_ID TRIGGER_REF SOURCE_TYPE SOURCE_RUN_ID WORKFLOW_URL ARTIFACT_REF RESOLVE_LOG_REF DOWNLOAD_LOG_REF MATERIALIZE_LOG_REF ANNOTATE_LOG_REF LOG_DIR; do
      require_env "$key"
    done
    emit <<EOF
sil4/tools/render_first_operational_evidence_packet.sh \\
  --output-dir ${OUTPUT_DIR} \\
  --track baseline \\
  --report-id ${REPORT_ID} \\
  --review-id ${REVIEW_ID} \\
  --execution-date ${EXECUTION_DATE} \\
  --commit-id ${COMMIT_ID} \\
  --ref-name ${REF_NAME} \\
  --run-id ${RUN_ID} \\
  --trigger-ref ${TRIGGER_REF} \\
  --source-type ${SOURCE_TYPE} \\
  --source-run-id ${SOURCE_RUN_ID} \\
  --workflow-url ${WORKFLOW_URL} \\
  --artifact-ref ${ARTIFACT_REF} \\
  --resolve-log-ref ${RESOLVE_LOG_REF} \\
  --download-log-ref ${DOWNLOAD_LOG_REF} \\
  --materialize-log-ref ${MATERIALIZE_LOG_REF} \\
  --annotate-log-ref ${ANNOTATE_LOG_REF} \\
  --log-dir ${LOG_DIR}
EOF
    ;;
  vendor)
    for key in OUTPUT_DIR REPORT_ID REVIEW_ID DATE TOOL_SOURCE COMMIT_ID REF_NAME RUN_ID JOB_NAME TRIGGER_REF ARTIFACT_NAME TOOL_VERSION RAW_EVIDENCE_TYPE RAW_EVIDENCE_LOCATION EXPORT_FORMAT CAPTURE_TIMESTAMP REVIEWER_ACCESS_PATH VENDOR_RULE_ID VENDOR_RULE_FAMILY SUBSET_ID SEVERITY FILE_PATH LOCATION INITIAL_DECISION WORKFLOW_URL RAW_ARTIFACT_REF VENDOR_REPORT_REF VENDOR_REVIEW_REF VENDOR_MATRIX_REF TRACKING_REF AUDIT_TRAIL_REF; do
      require_env "$key"
    done
    emit <<EOF
sil4/tools/render_first_operational_evidence_packet.sh \\
  --output-dir ${OUTPUT_DIR} \\
  --track vendor \\
  --report-id ${REPORT_ID} \\
  --review-id ${REVIEW_ID} \\
  --date ${DATE} \\
  --tool-source ${TOOL_SOURCE} \\
  --commit-id ${COMMIT_ID} \\
  --ref-name ${REF_NAME} \\
  --run-id ${RUN_ID} \\
  --job-name ${JOB_NAME} \\
  --trigger-ref ${TRIGGER_REF} \\
  --artifact-name ${ARTIFACT_NAME} \\
  --tool-version ${TOOL_VERSION} \\
  --raw-evidence-type ${RAW_EVIDENCE_TYPE} \\
  --raw-evidence-location ${RAW_EVIDENCE_LOCATION} \\
  --export-format ${EXPORT_FORMAT} \\
  --capture-timestamp ${CAPTURE_TIMESTAMP} \\
  --reviewer-access-path ${REVIEWER_ACCESS_PATH} \\
  --vendor-rule-id ${VENDOR_RULE_ID} \\
  --vendor-rule-family ${VENDOR_RULE_FAMILY} \\
  --subset-id ${SUBSET_ID} \\
  --severity ${SEVERITY} \\
  --file-path ${FILE_PATH} \\
  --location ${LOCATION} \\
  --initial-decision ${INITIAL_DECISION} \\
  --workflow-url ${WORKFLOW_URL} \\
  --raw-artifact-ref ${RAW_ARTIFACT_REF} \\
  --vendor-report-ref ${VENDOR_REPORT_REF} \\
  --vendor-review-ref ${VENDOR_REVIEW_REF} \\
  --vendor-matrix-ref ${VENDOR_MATRIX_REF} \\
  --tracking-ref ${TRACKING_REF} \\
  --audit-trail-ref ${AUDIT_TRAIL_REF}
EOF
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac

if [ -n "$OUTPUT" ]; then
  echo "Rendered operational packet invocation: $OUTPUT"
fi
