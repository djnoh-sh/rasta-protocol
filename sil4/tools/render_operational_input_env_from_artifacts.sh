#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_input_env_from_artifacts.sh \
  --track <baseline|vendor> \
  --output <env-file> \
  --output-dir <dir> \
  --report-id <id> \
  --review-id <id> \
  --artifact-dir <dir> \
  [track-specific args...]

baseline track extra args:
  --workflow-url <url>
  --artifact-ref <url-or-ref>
  --resolve-log-ref <text>
  --download-log-ref <text>
  --materialize-log-ref <text>
  --annotate-log-ref <text>

vendor track extra args:
  --workflow-url <url>
  --raw-artifact-ref <url-or-ref>
  --vendor-report-ref <path-or-url>
  --vendor-review-ref <path-or-url>
  --vendor-matrix-ref <path-or-url>
  --tracking-ref <path-or-url>
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

TRACK=""
OUTPUT=""
OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
ARTIFACT_DIR=""
FORWARD_ARGS=()

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --artifact-dir) ARTIFACT_DIR="$2"; shift 2 ;;
    *)
      FORWARD_ARGS+=("$1")
      shift
      if [ "$#" -gt 0 ]; then
        FORWARD_ARGS+=("$1")
        shift
      fi
      ;;
  esac
done

require_value "--track" "$TRACK"
require_value "--output" "$OUTPUT"
require_value "--output-dir" "$OUTPUT_DIR"
require_value "--report-id" "$REPORT_ID"
require_value "--review-id" "$REVIEW_ID"
require_value "--artifact-dir" "$ARTIFACT_DIR"

case "$TRACK" in
  baseline)
    "$SELF_DIR/render_baseline_fetch_input_env_from_ci_logs.sh" \
      --output "$OUTPUT" \
      --output-dir "$OUTPUT_DIR" \
      --report-id "$REPORT_ID" \
      --review-id "$REVIEW_ID" \
      --log-dir "$ARTIFACT_DIR" \
      "${FORWARD_ARGS[@]}"
    ;;
  vendor)
    "$SELF_DIR/render_vendor_input_env_from_export_dir.sh" \
      --output "$OUTPUT" \
      --output-dir "$OUTPUT_DIR" \
      --report-id "$REPORT_ID" \
      --review-id "$REVIEW_ID" \
      --export-dir "$ARTIFACT_DIR" \
      "${FORWARD_ARGS[@]}"
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac
