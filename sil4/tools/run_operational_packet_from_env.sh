#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: run_operational_packet_from_env.sh \
  --track <baseline|vendor> \
  --input <env-file> \
  [--execute]

Default mode is dry-run: validate input and print the generated command.
Use --execute to run the generated top-level packet helper command.
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
INPUT=""
EXECUTE="0"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --input) INPUT="$2"; shift 2 ;;
    --execute) EXECUTE="1"; shift 1 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--track" "$TRACK"
require_value "--input" "$INPUT"

TMP_CMD="$(mktemp /tmp/rsrx-operational-cmd.XXXXXX.sh)"
trap 'rm -f "$TMP_CMD"' EXIT

"$SELF_DIR/validate_operational_input_env.sh" \
  --track "$TRACK" \
  --input "$INPUT"

"$SELF_DIR/render_operational_packet_invocation.sh" \
  --track "$TRACK" \
  --input "$INPUT" \
  --output "$TMP_CMD" >/dev/null

echo "Generated command:"
cat "$TMP_CMD"

if [ "$EXECUTE" != "1" ]; then
  echo
  echo "Dry run only. Re-run with --execute to run the command."
  exit 0
fi

echo
echo "Executing generated command..."
/bin/bash "$TMP_CMD"
