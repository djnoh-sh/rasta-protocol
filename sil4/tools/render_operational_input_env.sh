#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_input_env.sh \
  --track <baseline|vendor> \
  [track-specific args...]

This helper delegates to:
  - render_baseline_fetch_input_env.sh
  - render_vendor_input_env.sh
  - render_vendor_input_env_from_export_metadata.sh
  - render_vendor_input_env_from_export_dir.sh
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
VENDOR_METADATA_MODE="0"
VENDOR_EXPORT_DIR_MODE="0"
PREV=""
FORWARD_ARGS=()

for arg in "$@"; do
  if [ "$PREV" = "--track" ]; then
    TRACK="$arg"
  fi
  if [ "$arg" = "--metadata-env" ]; then
    VENDOR_METADATA_MODE="1"
  fi
  if [ "$arg" = "--export-dir" ]; then
    VENDOR_EXPORT_DIR_MODE="1"
  fi
  PREV="$arg"
done

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) shift 2 ;;
    *)
      FORWARD_ARGS+=("$1")
      shift
      ;;
  esac
done

require_value "--track" "$TRACK"

case "$TRACK" in
  baseline)
    "$SELF_DIR/render_baseline_fetch_input_env.sh" "${FORWARD_ARGS[@]}"
    ;;
  vendor)
    if [ "$VENDOR_EXPORT_DIR_MODE" = "1" ]; then
      "$SELF_DIR/render_vendor_input_env_from_export_dir.sh" "${FORWARD_ARGS[@]}"
    elif [ "$VENDOR_METADATA_MODE" = "1" ]; then
      "$SELF_DIR/render_vendor_input_env_from_export_metadata.sh" "${FORWARD_ARGS[@]}"
    else
      "$SELF_DIR/render_vendor_input_env.sh" "${FORWARD_ARGS[@]}"
    fi
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac
