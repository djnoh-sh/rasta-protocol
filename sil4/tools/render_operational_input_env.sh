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

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    *) break ;;
  esac
done

require_value "--track" "$TRACK"

case "$TRACK" in
  baseline)
    "$SELF_DIR/render_baseline_fetch_input_env.sh" "$@"
    ;;
  vendor)
    "$SELF_DIR/render_vendor_input_env.sh" "$@"
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac
