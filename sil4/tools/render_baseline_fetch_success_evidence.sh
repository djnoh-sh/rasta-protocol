#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_baseline_fetch_success_evidence.sh \
  --output <report-md> \
  --report-id <id> \
  --execution-date <yyyy-mm-dd> \
  --commit-id <commit> \
  --ref-name <pr-or-branch> \
  --run-id <workflow-run-id> \
  --trigger-ref <git-ref> \
  --source-type <same-pr|main-branch> \
  --source-run-id <run-id> \
  --workflow-url <url> \
  --artifact-ref <url-or-ref> \
  --resolve-log-ref <text> \
  --download-log-ref <text> \
  --materialize-log-ref <text> \
  --annotate-log-ref <text> \
  [--event-type <event>] \
  [--job-name <job>] \
  [--log-dir <dir>]
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

OUTPUT=""
REPORT_ID=""
EXECUTION_DATE=""
COMMIT_ID=""
REF_NAME=""
RUN_ID=""
TRIGGER_REF=""
SOURCE_TYPE=""
SOURCE_RUN_ID=""
WORKFLOW_URL=""
ARTIFACT_REF=""
RESOLVE_LOG_REF=""
DOWNLOAD_LOG_REF=""
MATERIALIZE_LOG_REF=""
ANNOTATE_LOG_REF=""
EVENT_TYPE="pull_request"
JOB_NAME="sil4-verify"
LOG_DIR="/tmp/rsrx-ci-logs"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --execution-date) EXECUTION_DATE="$2"; shift 2 ;;
    --commit-id) COMMIT_ID="$2"; shift 2 ;;
    --ref-name) REF_NAME="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --trigger-ref) TRIGGER_REF="$2"; shift 2 ;;
    --source-type) SOURCE_TYPE="$2"; shift 2 ;;
    --source-run-id) SOURCE_RUN_ID="$2"; shift 2 ;;
    --workflow-url) WORKFLOW_URL="$2"; shift 2 ;;
    --artifact-ref) ARTIFACT_REF="$2"; shift 2 ;;
    --resolve-log-ref) RESOLVE_LOG_REF="$2"; shift 2 ;;
    --download-log-ref) DOWNLOAD_LOG_REF="$2"; shift 2 ;;
    --materialize-log-ref) MATERIALIZE_LOG_REF="$2"; shift 2 ;;
    --annotate-log-ref) ANNOTATE_LOG_REF="$2"; shift 2 ;;
    --event-type) EVENT_TYPE="$2"; shift 2 ;;
    --job-name) JOB_NAME="$2"; shift 2 ;;
    --log-dir) LOG_DIR="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--report-id" "$REPORT_ID"
require_value "--execution-date" "$EXECUTION_DATE"
require_value "--commit-id" "$COMMIT_ID"
require_value "--ref-name" "$REF_NAME"
require_value "--run-id" "$RUN_ID"
require_value "--trigger-ref" "$TRIGGER_REF"
require_value "--source-type" "$SOURCE_TYPE"
require_value "--source-run-id" "$SOURCE_RUN_ID"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--artifact-ref" "$ARTIFACT_REF"
require_value "--resolve-log-ref" "$RESOLVE_LOG_REF"
require_value "--download-log-ref" "$DOWNLOAD_LOG_REF"
require_value "--materialize-log-ref" "$MATERIALIZE_LOG_REF"
require_value "--annotate-log-ref" "$ANNOTATE_LOG_REF"

SUMMARY_ENV="$LOG_DIR/summary.env"
BASELINE_SUMMARY_ENV="$LOG_DIR/baseline_summary.env"
ANNOTATION_MD="$LOG_DIR/pr_annotation.md"
ANNOTATION_ENV="$LOG_DIR/pr_annotation.env"
BASELINE_ARTIFACT_ZIP="$LOG_DIR/baseline_artifact.zip"

CURRENT_SUMMARY_PRESENT="no"
BASELINE_SUMMARY_PRESENT="no"
PR_ANNOTATION_PRESENT="no"

if [ -f "$SUMMARY_ENV" ]; then
  CURRENT_SUMMARY_PRESENT="yes"
fi

if [ -f "$BASELINE_SUMMARY_ENV" ]; then
  BASELINE_SUMMARY_PRESENT="yes"
fi

if [ -f "$ANNOTATION_MD" ]; then
  PR_ANNOTATION_PRESENT="yes"
fi

ANNOTATION_BASELINE_MODE="$(get_env_value "$ANNOTATION_ENV" "ANNOTATION_BASELINE_MODE")"
ANNOTATION_DELTA_STATUS="$(get_env_value "$ANNOTATION_ENV" "ANNOTATION_DELTA_STATUS")"

if [ -z "$ANNOTATION_BASELINE_MODE" ]; then
  ANNOTATION_BASELINE_MODE="unknown"
fi

if [ -z "$ANNOTATION_DELTA_STATUS" ]; then
  ANNOTATION_DELTA_STATUS="unknown"
fi

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
# Baseline Fetch Success Evidence

## Document Control

- Report ID: \`$REPORT_ID\`
- Status: \`Draft\`
- Execution Date: \`$EXECUTION_DATE\`
- Workflow: \`.github/workflows/sil4-ci.yml\`
- Commit ID: \`$COMMIT_ID\`
- PR or Branch: \`$REF_NAME\`

## Purpose

이 문서는 \`sil4-ci\` workflow가 baseline artifact fetch를 실제로 성공했을 때 그 증빙을 기록한다.

## Runtime Context

| Field | Value |
| --- | --- |
| Event Type | \`$EVENT_TYPE\` |
| Workflow Run ID | \`$RUN_ID\` |
| Job Name | \`$JOB_NAME\` |
| Trigger Ref | \`$TRIGGER_REF\` |
| Artifact Name | \`sil4-ci-logs\` |

## Baseline Resolution Result

| Field | Value |
| --- | --- |
| Baseline Found | \`true\` |
| Source Type | \`$SOURCE_TYPE\` |
| Source Run ID | \`$SOURCE_RUN_ID\` |
| Materialized File | \`$BASELINE_SUMMARY_ENV\` |

## Verification Snapshot

| Field | Value |
| --- | --- |
| Current Summary Present | \`$CURRENT_SUMMARY_PRESENT\` |
| Baseline Summary Present | \`$BASELINE_SUMMARY_PRESENT\` |
| PR Annotation Present | \`$PR_ANNOTATION_PRESENT\` |
| Annotation Baseline Mode | \`$ANNOTATION_BASELINE_MODE\` |
| Annotation Delta Status | \`$ANNOTATION_DELTA_STATUS\` |

## Artifact References

- workflow URL: \`$WORKFLOW_URL\`
- artifact URL or run artifact reference: \`$ARTIFACT_REF\`
- \`summary.env\`: \`$SUMMARY_ENV\`
- \`baseline_artifact.zip\`: \`$BASELINE_ARTIFACT_ZIP\`
- \`baseline_summary.env\`: \`$BASELINE_SUMMARY_ENV\`
- \`pr_annotation.md\`: \`$ANNOTATION_MD\`

## Required Log Snippets

1. \`Resolve Baseline Artifact\`: \`$RESOLVE_LOG_REF\`
2. \`Download Baseline Artifact\`: \`$DOWNLOAD_LOG_REF\`
3. \`Materialize Baseline Summary\`: \`$MATERIALIZE_LOG_REF\`
4. \`Render PR Annotation\`: \`$ANNOTATE_LOG_REF\`

## Expected Confirmation Checklist

- [ ] prior artifact 식별 성공
- [ ] baseline artifact download 성공
- [ ] \`baseline_summary.env\` materialize 성공
- [ ] annotation mode가 \`delta-aware\`로 기록됨
- [ ] step summary 또는 PR comment에 annotation 게시됨

## Conclusion

- Runtime Evidence Status: \`TBD\`
- Follow-up Review: \`RV-TBD\`
EOF

echo "Rendered baseline fetch evidence: $OUTPUT"
