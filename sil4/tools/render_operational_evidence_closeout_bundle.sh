#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_closeout_bundle.sh \
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
  "$STARTER_WORK_DIR/readiness_update.md" \
  "$STARTER_WORK_DIR/readiness_tracker_rows.md" \
  "$STARTER_WORK_DIR/readiness_audit_note.md" \
  "$STARTER_WORK_DIR/runner.log"
do
  if [ ! -f "$path" ]; then
    echo "required closeout artifact missing: $path" >&2
    exit 1
  fi
done

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
### Operational Evidence Closeout Bundle

- starter work dir: \`$STARTER_WORK_DIR\`
- tracker-facing update: \`$STARTER_WORK_DIR/readiness_tracker_rows.md\`
- handoff-facing update: \`$STARTER_WORK_DIR/readiness_update.md\`
- audit-facing update: \`$STARTER_WORK_DIR/readiness_audit_note.md\`
- execution trace: \`$STARTER_WORK_DIR/runner.log\`

Closeout policy:

- use the tracker-facing update for \`EVS-001\` or \`EVS-003\`/\`EVS-004\` state transition before actual close
- use the audit-facing update as the pre-execution checkpoint until actual evidence links replace it
- keep the execution trace with the generated packet outputs for first operational evidence handoff
EOF

echo "Rendered operational evidence closeout bundle: $OUTPUT"
