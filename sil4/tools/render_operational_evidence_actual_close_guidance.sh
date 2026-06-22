#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_actual_close_guidance.sh \
  --output <snippet-md> \
  --summary <artifact-runner-summary.env>
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

OUTPUT=""
SUMMARY=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --summary) SUMMARY="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--summary" "$SUMMARY"

if [ ! -f "$SUMMARY" ]; then
  echo "summary file not found: $SUMMARY" >&2
  exit 1
fi

TRACK="$(get_env_value "$SUMMARY" "TRACK")"
OUTPUT_DIR="$(get_env_value "$SUMMARY" "OUTPUT_DIR")"
PACKET_MANIFEST="$(get_env_value "$SUMMARY" "PACKET_MANIFEST")"

require_value "TRACK" "$TRACK"
require_value "OUTPUT_DIR" "$OUTPUT_DIR"
require_value "PACKET_MANIFEST" "$PACKET_MANIFEST"

case "$TRACK" in
  baseline)
    TRACKER_ROW="$OUTPUT_DIR/baseline/baseline_tracker_row.md"
    AUDIT_UPDATE="$OUTPUT_DIR/baseline/baseline_audit_update.md"
    ;;
  vendor)
    TRACKER_ROW="$OUTPUT_DIR/vendor/vendor_tracker_row.md"
    AUDIT_UPDATE="$OUTPUT_DIR/vendor/vendor_audit_update.md"
    ;;
  *)
    echo "unsupported TRACK in summary: $TRACK" >&2
    exit 1
    ;;
esac

[ -f "$TRACKER_ROW" ]
[ -f "$AUDIT_UPDATE" ]
[ -f "$PACKET_MANIFEST" ]

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
### Operational Evidence Actual Close Guidance

- track: \`$TRACK\`
- packet manifest: \`$PACKET_MANIFEST\`
- tracker target: \`sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md\`
- tracker row artifact: \`$TRACKER_ROW\`
- audit target: \`sil4/docs/evidence/audit_trail_closeout.md\`
- audit update artifact: \`$AUDIT_UPDATE\`

Close guidance:

- paste the tracker row artifact into the execution tracker as the actual close/update candidate
- paste the audit update artifact into the audit trail landing zone as the actual evidence update candidate
- retain the packet manifest beside the closeout update for execution traceability
EOF

echo "Rendered operational evidence actual close guidance: $OUTPUT"
