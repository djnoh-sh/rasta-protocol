#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_input_env_from_artifacts.sh \
  --track <baseline|vendor|auto> \
  --output <env-file> \
  --output-dir <dir> \
  [--report-id <id>] \
  [--review-id <id>] \
  --artifact-dir <dir> \
  [track-specific args...]

baseline track extra args:
  --workflow-url <url>
  --artifact-ref <url-or-ref>
  --resolve-log-ref <text>
  --download-log-ref <text>
  --materialize-log-ref <text>
  --annotate-log-ref <text>

vendor track extra args:
  --workflow-url <url>
  --raw-artifact-ref <url-or-ref>
  --vendor-report-ref <path-or-url>
  --vendor-review-ref <path-or-url>
  --vendor-matrix-ref <path-or-url>
  --tracking-ref <path-or-url>
  --audit-trail-ref <path-or-url>
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

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

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

get_env_value() {
  local file="$1"
  local key="$2"
  local line
  if [ ! -f "$file" ]; then
    echo ""
    return
  fi
  line="$(grep -E "^${key}=" "$file" | tail -n 1 || true)"
  if [ -z "$line" ]; then
    echo ""
    return
  fi
  echo "${line#*=}"
}

default_report_id() {
  local track="$1"
  local artifact_dir="$2"
  case "$track" in
    baseline)
      local ctx="$artifact_dir/baseline_fetch_context.env"
      local date_value run_id
      date_value="$(get_env_value "$ctx" "EXECUTION_DATE")"
      run_id="$(get_env_value "$ctx" "RUN_ID")"
      require_value "EXECUTION_DATE in baseline_fetch_context.env" "$date_value"
      require_value "RUN_ID in baseline_fetch_context.env" "$run_id"
      printf 'EVID-CI-BLRUN-%s-%s' "${date_value//-/}" "$run_id"
      ;;
    vendor)
      local ctx="$artifact_dir/vendor_export_context.env"
      local date_value run_id
      date_value="$(get_env_value "$ctx" "DATE")"
      run_id="$(get_env_value "$ctx" "RUN_ID")"
      require_value "DATE in vendor_export_context.env" "$date_value"
      require_value "RUN_ID in vendor_export_context.env" "$run_id"
      printf 'EVID-CI-VDRUN-%s-%s' "${date_value//-/}" "$run_id"
      ;;
  esac
}

default_review_id() {
  local track="$1"
  local artifact_dir="$2"
  case "$track" in
    baseline)
      local ctx="$artifact_dir/baseline_fetch_context.env"
      local date_value run_id
      date_value="$(get_env_value "$ctx" "EXECUTION_DATE")"
      run_id="$(get_env_value "$ctx" "RUN_ID")"
      require_value "EXECUTION_DATE in baseline_fetch_context.env" "$date_value"
      require_value "RUN_ID in baseline_fetch_context.env" "$run_id"
      printf 'RV-BLRUN-%s-%s' "${date_value//-/}" "$run_id"
      ;;
    vendor)
      local ctx="$artifact_dir/vendor_export_context.env"
      local date_value run_id
      date_value="$(get_env_value "$ctx" "DATE")"
      run_id="$(get_env_value "$ctx" "RUN_ID")"
      require_value "DATE in vendor_export_context.env" "$date_value"
      require_value "RUN_ID in vendor_export_context.env" "$run_id"
      printf 'RV-VDRUN-%s-%s' "${date_value//-/}" "$run_id"
      ;;
  esac
}

TRACK=""
OUTPUT=""
OUTPUT_DIR=""
REPORT_ID=""
REVIEW_ID=""
ARTIFACT_DIR=""
FORWARD_ARGS=()

while [ "$#" -gt 0 ]; do
  case "$1" in
    --track) TRACK="$2"; shift 2 ;;
    --output) OUTPUT="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --artifact-dir) ARTIFACT_DIR="$2"; shift 2 ;;
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

require_value "--track" "$TRACK"
require_value "--output" "$OUTPUT"
require_value "--output-dir" "$OUTPUT_DIR"
require_value "--artifact-dir" "$ARTIFACT_DIR"

if [ "$TRACK" = "auto" ]; then
  TRACK="$(detect_track "$ARTIFACT_DIR")"
fi

if [ -z "$REPORT_ID" ]; then
  REPORT_ID="$(default_report_id "$TRACK" "$ARTIFACT_DIR")"
fi

if [ -z "$REVIEW_ID" ]; then
  REVIEW_ID="$(default_review_id "$TRACK" "$ARTIFACT_DIR")"
fi

case "$TRACK" in
  baseline)
    "$SELF_DIR/render_baseline_fetch_input_env_from_ci_logs.sh" \
      --output "$OUTPUT" \
      --output-dir "$OUTPUT_DIR" \
      --report-id "$REPORT_ID" \
      --review-id "$REVIEW_ID" \
      --log-dir "$ARTIFACT_DIR" \
      "${FORWARD_ARGS[@]}"
    ;;
  vendor)
    "$SELF_DIR/render_vendor_input_env_from_export_dir.sh" \
      --output "$OUTPUT" \
      --output-dir "$OUTPUT_DIR" \
      --report-id "$REPORT_ID" \
      --review-id "$REVIEW_ID" \
      --export-dir "$ARTIFACT_DIR" \
      "${FORWARD_ARGS[@]}"
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac
