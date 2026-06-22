#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_patch_guidance.sh \
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
    echo "required patch guidance input missing: $path" >&2
    exit 1
  fi
done

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
### Operational Evidence Patch Guidance

| Target Document | Starter Artifact | Use |
| --- | --- | --- |
| \`sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md\` | \`$STARTER_WORK_DIR/readiness_tracker_rows.md\` | paste pre-close state transition rows for \`EVS-001\`, \`EVS-003\`, \`EVS-004\` |
| \`sil4/docs/evidence/first_operational_evidence_handoff_sheet.md\` | \`$STARTER_WORK_DIR/readiness_update.md\` | paste readiness status note into the current execution handoff |
| \`sil4/docs/evidence/audit_trail_closeout.md\` | \`$STARTER_WORK_DIR/readiness_audit_note.md\` | paste pre-execution audit checkpoint until actual links replace it |
| generated packet output directory | \`$STARTER_WORK_DIR/runner.log\` | retain execution trace with packet/summary/receipt artifacts |

Patch policy:

- apply tracker and audit updates before marking any actual close status
- keep runner trace adjacent to generated packet outputs for first operational evidence review
EOF

echo "Rendered operational evidence patch guidance: $OUTPUT"
