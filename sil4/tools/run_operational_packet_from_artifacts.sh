#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: run_operational_packet_from_artifacts.sh \
  --track <baseline|vendor|auto> \
  --input <generated-env-file> \
  --artifact-dir <dir> \
  --output-dir <dir> \
  [--report-id <id>] \
  [--review-id <id>] \
  [track-specific args...]

This helper:
  1. renders packet env input from artifact dir
  2. runs run_operational_packet_from_env.sh --execute
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

detect_track() {
  local artifact_dir="$1"
  if [ -f "$artifact_dir/baseline_fetch_context.env" ]; then
    echo "baseline"
    return 0
  fi
  if [ -f "$artifact_dir/vendor_export_context.env" ]; then
    echo "vendor"
    return 0
  fi
  echo "unable to detect track from artifact dir: $artifact_dir" >&2
  exit 1
}

TRACK=""
INPUT=""
ARTIFACT_DIR=""
OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
FORWARD_ARGS=()

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --input) INPUT="$2"; shift 2 ;;
    --artifact-dir) ARTIFACT_DIR="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
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
require_value "--input" "$INPUT"
require_value "--artifact-dir" "$ARTIFACT_DIR"
require_value "--output-dir" "$OUTPUT_DIR"

if [ "$TRACK" = "auto" ]; then
  TRACK="$(detect_track "$ARTIFACT_DIR")"
fi

"$SELF_DIR/render_operational_input_env_from_artifacts.sh" \
  --track "$TRACK" \
  --output "$INPUT" \
  --output-dir "$OUTPUT_DIR" \
  --report-id "$REPORT_ID" \
  --review-id "$REVIEW_ID" \
  --artifact-dir "$ARTIFACT_DIR" \
  "${FORWARD_ARGS[@]}"

"$SELF_DIR/run_operational_packet_from_env.sh" \
  --track "$TRACK" \
  --input "$INPUT" \
  --execute
