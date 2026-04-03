#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: run_operational_evidence_from_artifact.sh \
  --artifact-dir <dir> \
  [--track <baseline|vendor|auto>] \
  [--work-dir <dir>] \
  [artifact-runner args...]

This helper:
  1. runs readiness preflight for the given artifact dir
  2. renders readiness update/commands/tracker-row/audit-note artifacts
  3. runs the artifact-dir packet runner
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

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
WORK_DIR="/tmp/rsrx-operational-evidence-starter"
TRACK="auto"
ARTIFACT_DIR=""
FORWARD_ARGS=()

detect_track() {
  local artifact_dir="$1"
  if [ -f "$artifact_dir/baseline_fetch_context.env" ]; then
    echo "baseline"
    return 0
  fi
  if [ -f "$artifact_dir/vendor_export_context.env" ]; then
    echo "vendor"
    return 0
  fi
  echo "unable to detect track from artifact dir: $artifact_dir" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --artifact-dir) ARTIFACT_DIR="$2"; shift 2 ;;
    --track) TRACK="$2"; shift 2 ;;
    --work-dir) WORK_DIR="$2"; shift 2 ;;
    *)
      FORWARD_ARGS+=("$1")
      shift
      if [ "$#" -gt 0 ]; then
        FORWARD_ARGS+=("$1")
        shift
      fi
      ;;
  esac
done

require_value "--artifact-dir" "$ARTIFACT_DIR"

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

READINESS_DIR="$WORK_DIR/readiness"
READINESS_UPDATE_MD="$WORK_DIR/readiness_update.md"
READY_COMMANDS_MD="$WORK_DIR/ready_commands.md"
READINESS_TRACKER_ROWS_MD="$WORK_DIR/readiness_tracker_rows.md"
READINESS_AUDIT_NOTE_MD="$WORK_DIR/readiness_audit_note.md"
RUNNER_LOG="$WORK_DIR/runner.log"
READINESS_BASELINE_DIR=""
READINESS_VENDOR_DIR=""

if [ "$TRACK" = "auto" ]; then
  TRACK="$(detect_track "$ARTIFACT_DIR")"
fi

case "$TRACK" in
  baseline) READINESS_BASELINE_DIR="$ARTIFACT_DIR" ;;
  vendor) READINESS_VENDOR_DIR="$ARTIFACT_DIR" ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac

"$SELF_DIR/check_operational_evidence_readiness.sh" \
  --work-dir "$READINESS_DIR" \
  --baseline-artifact-dir "$READINESS_BASELINE_DIR" \
  --vendor-artifact-dir "$READINESS_VENDOR_DIR" >/dev/null

case "$TRACK" in
  baseline)
    [ "$(read_env_value "$READINESS_DIR/summary.env" BASELINE_ARTIFACT_STATUS)" = "Available" ] || {
      echo "baseline artifact readiness is blocked" >&2
      exit 1
    }
    ;;
  vendor)
    [ "$(read_env_value "$READINESS_DIR/summary.env" VENDOR_ARTIFACT_STATUS)" = "Available" ] || {
      echo "vendor artifact readiness is blocked" >&2
      exit 1
    }
    ;;
esac

"$SELF_DIR/render_operational_evidence_readiness_update.sh" \
  --output "$READINESS_UPDATE_MD" \
  --summary-env "$READINESS_DIR/summary.env" \
  --tracker-ref "sil4/docs/evidence/first_actual_vendor_evidence_set_execution_tracker.md" \
  --handoff-ref "sil4/docs/evidence/first_operational_evidence_handoff_sheet.md" \
  --execution-packet-ref "sil4/docs/evidence/first_operational_evidence_execution_packet.md" >/dev/null

"$SELF_DIR/render_operational_evidence_ready_commands.sh" \
  --output "$READY_COMMANDS_MD" \
  --summary-env "$READINESS_DIR/summary.env" >/dev/null

"$SELF_DIR/render_operational_evidence_readiness_tracker_rows.sh" \
  --output "$READINESS_TRACKER_ROWS_MD" \
  --summary-env "$READINESS_DIR/summary.env" >/dev/null

"$SELF_DIR/render_operational_evidence_readiness_audit_note.sh" \
  --output "$READINESS_AUDIT_NOTE_MD" \
  --summary-env "$READINESS_DIR/summary.env" \
  --audit-ref "sil4/docs/evidence/audit_trail_closeout.md" >/dev/null

"$SELF_DIR/run_operational_packet_from_artifacts.sh" \
  --track "$TRACK" \
  --artifact-dir "$ARTIFACT_DIR" \
  "${FORWARD_ARGS[@]}" >"$RUNNER_LOG"

echo "Operational evidence starter work dir: $WORK_DIR"
echo "Readiness summary: $READINESS_DIR/summary.md"
echo "Readiness update: $READINESS_UPDATE_MD"
echo "Ready commands: $READY_COMMANDS_MD"
echo "Readiness tracker rows: $READINESS_TRACKER_ROWS_MD"
echo "Readiness audit note: $READINESS_AUDIT_NOTE_MD"
echo "Runner log: $RUNNER_LOG"
