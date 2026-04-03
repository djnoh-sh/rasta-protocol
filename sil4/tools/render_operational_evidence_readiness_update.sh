#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_readiness_update.sh \
  --output <snippet-md> \
  --summary-env <summary.env> \
  --tracker-ref <path-or-url> \
  --handoff-ref <path-or-url> \
  [--execution-packet-ref <path-or-url>]
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

read_env_value() {
  local env_file="$1"
  local key="$2"
  local line

  line="$(grep -m1 "^${key}=" "$env_file" || true)"
  if [ -z "$line" ]; then
    echo "missing required key in summary env: $key" >&2
    exit 1
  fi
  printf '%s\n' "${line#*=}"
}

OUTPUT=""
SUMMARY_ENV=""
TRACKER_REF=""
HANDOFF_REF=""
EXECUTION_PACKET_REF=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --summary-env) SUMMARY_ENV="$2"; shift 2 ;;
    --tracker-ref) TRACKER_REF="$2"; shift 2 ;;
    --handoff-ref) HANDOFF_REF="$2"; shift 2 ;;
    --execution-packet-ref) EXECUTION_PACKET_REF="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--summary-env" "$SUMMARY_ENV"
require_value "--tracker-ref" "$TRACKER_REF"
require_value "--handoff-ref" "$HANDOFF_REF"

if [ ! -f "$SUMMARY_ENV" ]; then
  echo "summary env not found: $SUMMARY_ENV" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"

OVERALL_STATUS="$(read_env_value "$SUMMARY_ENV" OVERALL_READINESS_STATUS)"
BASELINE_STATUS="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_STATUS)"
BASELINE_NOTE="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_NOTE)"
VENDOR_STATUS="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_STATUS)"
VENDOR_NOTE="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_NOTE)"

cat >"$OUTPUT" <<EOF
### Operational Evidence Readiness Update

- overall readiness: \`$OVERALL_STATUS\`
- baseline availability: \`$BASELINE_STATUS\` - $BASELINE_NOTE
- vendor availability: \`$VENDOR_STATUS\` - $VENDOR_NOTE
- execution tracker target: \`$TRACKER_REF\`
- handoff target: \`$HANDOFF_REF\`
EOF

if [ -n "$EXECUTION_PACKET_REF" ]; then
  cat >>"$OUTPUT" <<EOF
- packet target: \`$EXECUTION_PACKET_REF\`
EOF
fi

cat >>"$OUTPUT" <<'EOF'

Next update policy:

- if overall readiness is `Ready`, start the matching artifact-dir packet run and paste the resulting packet/summary/receipt refs into the tracker and audit update path
- if overall readiness is not `Ready`, keep tracker items open and update the handoff sheet with the missing artifact note only
EOF

echo "Rendered operational evidence readiness update: $OUTPUT"
