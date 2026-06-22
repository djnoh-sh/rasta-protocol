#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: validate_first_operational_packet.sh \
  --track <baseline|vendor> \
  --output-dir <dir>
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

require_file() {
  local path="$1"
  if [ ! -s "$path" ]; then
    echo "missing or empty required file: $path" >&2
    exit 1
  fi
}

TRACK=""
OUTPUT_DIR=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--track" "$TRACK"
require_value "--output-dir" "$OUTPUT_DIR"

require_file "$OUTPUT_DIR/packet_manifest.md"

case "$TRACK" in
  baseline)
    require_file "$OUTPUT_DIR/baseline/baseline_runtime_report.md"
    require_file "$OUTPUT_DIR/baseline/baseline_runtime_review.md"
    require_file "$OUTPUT_DIR/baseline/baseline_tracker_row.md"
    require_file "$OUTPUT_DIR/baseline/baseline_audit_update.md"
    ;;
  vendor)
    require_file "$OUTPUT_DIR/vendor/vendor_runtime_report.md"
    require_file "$OUTPUT_DIR/vendor/vendor_runtime_review.md"
    require_file "$OUTPUT_DIR/vendor/vendor_matrix_row.md"
    require_file "$OUTPUT_DIR/vendor/vendor_tracker_row.md"
    require_file "$OUTPUT_DIR/vendor/vendor_audit_update.md"
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac

echo "Validated operational packet output: $OUTPUT_DIR ($TRACK)"
