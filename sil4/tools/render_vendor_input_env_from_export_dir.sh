#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_vendor_input_env_from_export_dir.sh \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --export-dir <dir> \
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

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

OUTPUT=""
OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
EXPORT_DIR=""
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
    --export-dir) EXPORT_DIR="$2"; shift 2 ;;
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
require_value "--export-dir" "$EXPORT_DIR"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--raw-artifact-ref" "$RAW_ARTIFACT_REF"
require_value "--vendor-report-ref" "$VENDOR_REPORT_REF"
require_value "--vendor-review-ref" "$VENDOR_REVIEW_REF"
require_value "--vendor-matrix-ref" "$VENDOR_MATRIX_REF"
require_value "--tracking-ref" "$TRACKING_REF"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"

METADATA_ENV="$EXPORT_DIR/vendor_export_context.env"

if [ ! -f "$METADATA_ENV" ]; then
  echo "vendor export context not found: $METADATA_ENV" >&2
  exit 1
fi

"$SELF_DIR/render_vendor_input_env_from_export_metadata.sh" \
  --output "$OUTPUT" \
  --output-dir "$OUTPUT_DIR" \
  --report-id "$REPORT_ID" \
  --review-id "$REVIEW_ID" \
  --metadata-env "$METADATA_ENV" \
  --workflow-url "$WORKFLOW_URL" \
  --raw-artifact-ref "$RAW_ARTIFACT_REF" \
  --vendor-report-ref "$VENDOR_REPORT_REF" \
  --vendor-review-ref "$VENDOR_REVIEW_REF" \
  --vendor-matrix-ref "$VENDOR_MATRIX_REF" \
  --tracking-ref "$TRACKING_REF" \
  --audit-trail-ref "$AUDIT_TRAIL_REF"
