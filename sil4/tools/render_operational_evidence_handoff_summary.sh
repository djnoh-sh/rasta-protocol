#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_handoff_summary.sh \
  --output <snippet-md> \
  --starter-work-dir <dir>
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
STARTER_WORK_DIR=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --starter-work-dir) STARTER_WORK_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--starter-work-dir" "$STARTER_WORK_DIR"

if [ ! -d "$STARTER_WORK_DIR" ]; then
  echo "starter work dir not found: $STARTER_WORK_DIR" >&2
  exit 1
fi

for path in \
  "$STARTER_WORK_DIR/readiness/summary.md" \
  "$STARTER_WORK_DIR/readiness_update.md" \
  "$STARTER_WORK_DIR/ready_commands.md" \
  "$STARTER_WORK_DIR/readiness_tracker_rows.md" \
  "$STARTER_WORK_DIR/readiness_audit_note.md" \
  "$STARTER_WORK_DIR/runner.log"
do
  if [ ! -f "$path" ]; then
    echo "required starter artifact missing: $path" >&2
    exit 1
  fi
done

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
### Operational Evidence Handoff Summary

- starter work dir: \`$STARTER_WORK_DIR\`
- readiness summary: \`$STARTER_WORK_DIR/readiness/summary.md\`
- readiness update: \`$STARTER_WORK_DIR/readiness_update.md\`
- ready commands: \`$STARTER_WORK_DIR/ready_commands.md\`
- tracker rows: \`$STARTER_WORK_DIR/readiness_tracker_rows.md\`
- audit note: \`$STARTER_WORK_DIR/readiness_audit_note.md\`
- runner log: \`$STARTER_WORK_DIR/runner.log\`

Handoff policy:

- attach the readiness update, tracker rows, and audit note before or with the first actual execution handoff
- use the ready commands and runner log as the operator-facing execution entry and trace
EOF

echo "Rendered operational evidence handoff summary: $OUTPUT"
