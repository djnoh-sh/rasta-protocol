#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_readiness_audit_note.sh \
  --output <snippet-md> \
  --summary-env <summary.env> \
  --audit-ref <path-or-url>
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
AUDIT_REF=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --summary-env) SUMMARY_ENV="$2"; shift 2 ;;
    --audit-ref) AUDIT_REF="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--summary-env" "$SUMMARY_ENV"
require_value "--audit-ref" "$AUDIT_REF"

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
### Operational Evidence Readiness Audit Note

- audit target: \`$AUDIT_REF\`
- overall readiness: \`$OVERALL_STATUS\`
- baseline pre-execution status: \`$BASELINE_STATUS\` - $BASELINE_NOTE
- vendor pre-execution status: \`$VENDOR_STATUS\` - $VENDOR_NOTE

Audit note policy:

- if overall readiness is \`Ready\`, keep this note as the pre-execution checkpoint and replace it with actual evidence links after packet execution
- if overall readiness is not \`Ready\`, retain this note and do not promote audit state beyond artifact-waiting
EOF

echo "Rendered operational evidence readiness audit note: $OUTPUT"
