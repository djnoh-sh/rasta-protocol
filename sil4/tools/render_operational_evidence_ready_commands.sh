#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_ready_commands.sh \
  --output <snippet-md> \
  --summary-env <summary.env>
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

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --summary-env) SUMMARY_ENV="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--summary-env" "$SUMMARY_ENV"

if [ ! -f "$SUMMARY_ENV" ]; then
  echo "summary env not found: $SUMMARY_ENV" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"

OVERALL_STATUS="$(read_env_value "$SUMMARY_ENV" OVERALL_READINESS_STATUS)"
BASELINE_STATUS="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_STATUS)"
BASELINE_DIR="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_DIR)"
VENDOR_STATUS="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_STATUS)"
VENDOR_DIR="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_DIR)"

cat >"$OUTPUT" <<EOF
### Operational Evidence Ready Commands

- overall readiness: \`$OVERALL_STATUS\`
EOF

if [ "$BASELINE_STATUS" = "Available" ]; then
  cat >>"$OUTPUT" <<EOF

Baseline command:

\`\`\`bash
sil4/tools/run_operational_packet_from_artifacts.sh --track auto --artifact-dir $BASELINE_DIR
\`\`\`
EOF
else
  cat >>"$OUTPUT" <<'EOF'

Baseline command:

- blocked: baseline artifact dir is not ready
EOF
fi

if [ "$VENDOR_STATUS" = "Available" ]; then
  cat >>"$OUTPUT" <<EOF

Vendor command:

\`\`\`bash
sil4/tools/run_operational_packet_from_artifacts.sh --track auto --artifact-dir $VENDOR_DIR
\`\`\`
EOF
else
  cat >>"$OUTPUT" <<'EOF'

Vendor command:

- blocked: vendor artifact dir is not ready
EOF
fi

echo "Rendered operational evidence ready commands: $OUTPUT"
