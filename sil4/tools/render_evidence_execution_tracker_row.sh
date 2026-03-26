#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_evidence_execution_tracker_row.sh \
  --output <row-md> \
  --item-id <EVS-xxx> \
  --status <Open|Closed|In Progress> \
  --target-artifact <artifact> \
  --execution-note <note>
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

OUTPUT=""
ITEM_ID=""
STATUS_VALUE=""
TARGET_ARTIFACT=""
EXECUTION_NOTE=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --item-id) ITEM_ID="$2"; shift 2 ;;
    --status) STATUS_VALUE="$2"; shift 2 ;;
    --target-artifact) TARGET_ARTIFACT="$2"; shift 2 ;;
    --execution-note) EXECUTION_NOTE="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--item-id" "$ITEM_ID"
require_value "--status" "$STATUS_VALUE"
require_value "--target-artifact" "$TARGET_ARTIFACT"
require_value "--execution-note" "$EXECUTION_NOTE"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
| $ITEM_ID | \`$STATUS_VALUE\` | $TARGET_ARTIFACT | $EXECUTION_NOTE |
EOF

echo "Rendered evidence execution tracker row: $OUTPUT"
