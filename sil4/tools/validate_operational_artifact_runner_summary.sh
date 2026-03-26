#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: validate_operational_artifact_runner_summary.sh --summary <summary-env>
EOF
  exit 1
}

require_value() {
  local name="$1"
  local value="$2"
  if [ -z "$value" ]; then
    echo "missing required value: $name" >&2
    exit 1
  fi
}

get_env_value() {
  local file="$1"
  local key="$2"
  local line
  line="$(grep -E "^${key}=" "$file" | tail -n 1 || true)"
  if [ -z "$line" ]; then
    echo ""
    return
  fi
  echo "${line#*=}"
}

SUMMARY=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --summary) SUMMARY="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--summary" "$SUMMARY"

if [ ! -f "$SUMMARY" ]; then
  echo "summary file not found: $SUMMARY" >&2
  exit 1
fi

TRACK="$(get_env_value "$SUMMARY" "TRACK")"
ARTIFACT_DIR="$(get_env_value "$SUMMARY" "ARTIFACT_DIR")"
OUTPUT_DIR="$(get_env_value "$SUMMARY" "OUTPUT_DIR")"
INPUT_ENV="$(get_env_value "$SUMMARY" "INPUT_ENV")"
PACKET_MANIFEST="$(get_env_value "$SUMMARY" "PACKET_MANIFEST")"

require_value "TRACK" "$TRACK"
require_value "ARTIFACT_DIR" "$ARTIFACT_DIR"
require_value "OUTPUT_DIR" "$OUTPUT_DIR"
require_value "INPUT_ENV" "$INPUT_ENV"
require_value "PACKET_MANIFEST" "$PACKET_MANIFEST"

case "$TRACK" in
  baseline|vendor) ;;
  *)
    echo "unsupported TRACK in summary: $TRACK" >&2
    exit 1
    ;;
esac

[ -d "$ARTIFACT_DIR" ]
[ -d "$OUTPUT_DIR" ]
[ -f "$INPUT_ENV" ]
[ -f "$PACKET_MANIFEST" ]

echo "Operational artifact runner summary validated: $SUMMARY"
