#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_vendor_matrix_actual_row.sh \
  --output <row-md> \
  --tool-source <tool-name> \
  --run-id <workflow-or-capture-id> \
  --rule-id <rule-id> \
  --rule-family <family> \
  --subset-id <subset> \
  --severity <severity> \
  --file-path <file> \
  --location <line-or-range> \
  --initial-decision <decision> \
  --report-link <path-or-url> \
  --review-link <path-or-url> \
  --tracking-link <path-or-url> \
  --status <status>
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
TOOL_SOURCE=""
RUN_ID=""
RULE_ID=""
RULE_FAMILY=""
SUBSET_ID=""
SEVERITY=""
FILE_PATH=""
LOCATION=""
INITIAL_DECISION=""
REPORT_LINK=""
REVIEW_LINK=""
TRACKING_LINK=""
STATUS_VALUE=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --tool-source) TOOL_SOURCE="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --rule-id) RULE_ID="$2"; shift 2 ;;
    --rule-family) RULE_FAMILY="$2"; shift 2 ;;
    --subset-id) SUBSET_ID="$2"; shift 2 ;;
    --severity) SEVERITY="$2"; shift 2 ;;
    --file-path) FILE_PATH="$2"; shift 2 ;;
    --location) LOCATION="$2"; shift 2 ;;
    --initial-decision) INITIAL_DECISION="$2"; shift 2 ;;
    --report-link) REPORT_LINK="$2"; shift 2 ;;
    --review-link) REVIEW_LINK="$2"; shift 2 ;;
    --tracking-link) TRACKING_LINK="$2"; shift 2 ;;
    --status) STATUS_VALUE="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--tool-source" "$TOOL_SOURCE"
require_value "--run-id" "$RUN_ID"
require_value "--rule-id" "$RULE_ID"
require_value "--rule-family" "$RULE_FAMILY"
require_value "--subset-id" "$SUBSET_ID"
require_value "--severity" "$SEVERITY"
require_value "--file-path" "$FILE_PATH"
require_value "--location" "$LOCATION"
require_value "--initial-decision" "$INITIAL_DECISION"
require_value "--report-link" "$REPORT_LINK"
require_value "--review-link" "$REVIEW_LINK"
require_value "--tracking-link" "$TRACKING_LINK"
require_value "--status" "$STATUS_VALUE"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
| \`$TOOL_SOURCE\` | \`$RUN_ID\` | \`$RULE_ID\` | \`$RULE_FAMILY\` | \`$SUBSET_ID\` | \`$SEVERITY\` | \`$FILE_PATH\` | \`$LOCATION\` | \`$INITIAL_DECISION\` | \`$REPORT_LINK\` | \`$REVIEW_LINK\` | \`$TRACKING_LINK\` | \`$STATUS_VALUE\` |
EOF

echo "Rendered vendor matrix actual row: $OUTPUT"
