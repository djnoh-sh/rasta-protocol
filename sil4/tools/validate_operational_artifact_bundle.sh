#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: validate_operational_artifact_bundle.sh \
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

TRACK=""
OUTPUT_DIR=""
SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--track" "$TRACK"
require_value "--output-dir" "$OUTPUT_DIR"

bash "$SELF_DIR/validate_first_operational_packet.sh" \
  --track "$TRACK" \
  --output-dir "$OUTPUT_DIR" >/dev/null

bash "$SELF_DIR/validate_operational_artifact_runner_summary.sh" \
  --summary "$OUTPUT_DIR/artifact_runner_summary.env" >/dev/null

bash "$SELF_DIR/validate_operational_artifact_runner_receipt.sh" \
  --receipt "$OUTPUT_DIR/artifact_runner_receipt.md" >/dev/null

echo "Operational artifact bundle validated: $OUTPUT_DIR ($TRACK)"
