#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: check_operational_evidence_readiness.sh \
  [--work-dir <dir>] \
  [--baseline-artifact-dir <dir>] \
  [--vendor-artifact-dir <dir>] \
  [--require-ready]

This helper:
  1. checks whether actual baseline/vendor artifacts are available
  2. writes readiness summary.md and summary.env
  3. optionally fails if both tracks are not ready
EOF
  exit 1
}

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
WORK_DIR="/tmp/rsrx-operational-readiness"
BASELINE_ARTIFACT_DIR=""
VENDOR_ARTIFACT_DIR=""
REQUIRE_READY="0"

. "$SELF_DIR/verification_sequence_common.sh"

availability_state() {
  local track="$1"
  local artifact_dir="$2"
  local context_file="$3"

  if [ -z "$artifact_dir" ]; then
    echo "Missing"
    return 0
  fi
  if [ ! -d "$artifact_dir" ]; then
    echo "Missing"
    return 0
  fi
  if [ ! -f "$artifact_dir/$context_file" ]; then
    echo "Missing"
    return 0
  fi
  echo "Available"
}

availability_note() {
  local track="$1"
  local artifact_dir="$2"
  local context_file="$3"

  if [ -z "$artifact_dir" ]; then
    case "$track" in
      baseline) echo "actual successful PR baseline artifact 대기" ;;
      vendor) echo "actual vendor export artifact 대기" ;;
    esac
    return 0
  fi
  if [ ! -d "$artifact_dir" ]; then
    echo "artifact dir not found: $artifact_dir"
    return 0
  fi
  if [ ! -f "$artifact_dir/$context_file" ]; then
    echo "required context file missing: $artifact_dir/$context_file"
    return 0
  fi
  echo "ready via $artifact_dir/$context_file"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --work-dir) WORK_DIR="$2"; shift 2 ;;
    --baseline-artifact-dir) BASELINE_ARTIFACT_DIR="$2"; shift 2 ;;
    --vendor-artifact-dir) VENDOR_ARTIFACT_DIR="$2"; shift 2 ;;
    --require-ready) REQUIRE_READY="1"; shift ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

SUMMARY_MD="$WORK_DIR/summary.md"
SUMMARY_ENV="$WORK_DIR/summary.env"

BASELINE_STATE="$(availability_state baseline "$BASELINE_ARTIFACT_DIR" baseline_fetch_context.env)"
VENDOR_STATE="$(availability_state vendor "$VENDOR_ARTIFACT_DIR" vendor_export_context.env)"
BASELINE_NOTE="$(availability_note baseline "$BASELINE_ARTIFACT_DIR" baseline_fetch_context.env)"
VENDOR_NOTE="$(availability_note vendor "$VENDOR_ARTIFACT_DIR" vendor_export_context.env)"

OVERALL_STATUS="Blocked"
if [ "$BASELINE_STATE" = "Available" ] && [ "$VENDOR_STATE" = "Available" ]; then
  OVERALL_STATUS="Ready"
fi

mark_verification_phase_complete "$WORK_DIR" setup
mark_verification_phase_complete "$WORK_DIR" check

cat >"$SUMMARY_MD" <<EOF
# Operational Evidence Readiness Summary

| Track | Availability | Note |
| --- | --- | --- |
| Baseline Fetch | $BASELINE_STATE | $BASELINE_NOTE |
| Vendor Export | $VENDOR_STATE | $VENDOR_NOTE |
| Overall | $OVERALL_STATUS | ready only when both track artifacts are available |

## Phase Markers

| Phase | Marker |
| --- | --- |
| Setup | \`$(phase_marker_path "$WORK_DIR" setup)\` |
| Check | \`$(phase_marker_path "$WORK_DIR" check)\` |

## Suggested Next Commands

EOF

if [ "$BASELINE_STATE" = "Available" ]; then
  cat >>"$SUMMARY_MD" <<EOF
- baseline:
  \`sil4/tools/run_operational_packet_from_artifacts.sh --track auto --artifact-dir $BASELINE_ARTIFACT_DIR\`
EOF
else
  cat >>"$SUMMARY_MD" <<'EOF'
- baseline:
  wait for actual successful PR baseline artifact dir with `baseline_fetch_context.env`
EOF
fi

if [ "$VENDOR_STATE" = "Available" ]; then
  cat >>"$SUMMARY_MD" <<EOF
- vendor:
  \`sil4/tools/run_operational_packet_from_artifacts.sh --track auto --artifact-dir $VENDOR_ARTIFACT_DIR\`
EOF
else
  cat >>"$SUMMARY_MD" <<'EOF'
- vendor:
  wait for actual vendor export dir with `vendor_export_context.env`
EOF
fi

cat >"$SUMMARY_ENV" <<EOF
SUMMARY_MD=$SUMMARY_MD
OVERALL_READINESS_STATUS=$OVERALL_STATUS
BASELINE_ARTIFACT_STATUS=$BASELINE_STATE
BASELINE_ARTIFACT_NOTE=$BASELINE_NOTE
BASELINE_ARTIFACT_DIR=$BASELINE_ARTIFACT_DIR
VENDOR_ARTIFACT_STATUS=$VENDOR_STATE
VENDOR_ARTIFACT_NOTE=$VENDOR_NOTE
VENDOR_ARTIFACT_DIR=$VENDOR_ARTIFACT_DIR
SETUP_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" setup)
CHECK_PHASE_MARKER=$(phase_marker_path "$WORK_DIR" check)
EOF

echo "Operational evidence readiness summary: $SUMMARY_MD"

if [ "$REQUIRE_READY" = "1" ] && [ "$OVERALL_STATUS" != "Ready" ]; then
  echo "operational evidence readiness is blocked" >&2
  exit 1
fi
