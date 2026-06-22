#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_readiness_tracker_rows.sh \
  --output <rows-md> \
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

render_row() {
  local item_id="$1"
  local target_artifact="$2"
  local status_value="$3"
  local execution_note="$4"

  printf '| %s | `%s` | %s | %s |\n' \
    "$item_id" "$status_value" "$target_artifact" "$execution_note"
}

baseline_row_status() {
  local baseline_status="$1"
  if [ "$baseline_status" = "Available" ]; then
    echo "In Progress"
  else
    echo "Open"
  fi
}

vendor_row_status() {
  local vendor_status="$1"
  if [ "$vendor_status" = "Available" ]; then
    echo "In Progress"
  else
    echo "Open"
  fi
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

BASELINE_STATUS="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_STATUS)"
BASELINE_NOTE="$(read_env_value "$SUMMARY_ENV" BASELINE_ARTIFACT_NOTE)"
VENDOR_STATUS="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_STATUS)"
VENDOR_NOTE="$(read_env_value "$SUMMARY_ENV" VENDOR_ARTIFACT_NOTE)"

cat >"$OUTPUT" <<EOF
### Operational Evidence Readiness Tracker Rows

| Item ID | Current Status | Target Artifact | Execution Note |
| --- | --- | --- | --- |
$(render_row "EVS-001" "baseline fetch success runtime evidence" "$(baseline_row_status "$BASELINE_STATUS")" "$BASELINE_NOTE")
$(render_row "EVS-003" "vendor raw evidence reference" "$(vendor_row_status "$VENDOR_STATUS")" "$VENDOR_NOTE")
$(render_row "EVS-004" "vendor finding report" "$(vendor_row_status "$VENDOR_STATUS")" "$VENDOR_NOTE")
EOF

echo "Rendered operational evidence readiness tracker rows: $OUTPUT"
