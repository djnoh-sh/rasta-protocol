#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_baseline_fetch_runtime_review.sh \
  --output <review-md> \
  --review-id <id> \
  --date <yyyy-mm-dd> \
  --report-ref <path-or-url> \
  --runtime-report-id <evidence-run-id> \
  --commit-id <commit> \
  --trigger-ref <ref> \
  --run-id <workflow-run-id> \
  --source-type <same-pr|main-branch> \
  --source-run-id <run-id> \
  --materialized-file <path> \
  --resolve-log-ref <text> \
  --download-log-ref <text> \
  --materialize-log-ref <text> \
  --annotate-log-ref <text> \
  --annotation-mode <mode> \
  --audit-trail-ref <path-or-url> \
  --index-ref <path-or-url> \
  --roadmap-ref <path-or-url> \
  [--reviewer <name>] \
  [--result <result>] \
  [--summary <text>]
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
REVIEW_ID=""
DATE_VALUE=""
REPORT_REF=""
RUNTIME_REPORT_ID=""
COMMIT_ID=""
TRIGGER_REF=""
RUN_ID=""
SOURCE_TYPE=""
SOURCE_RUN_ID=""
MATERIALIZED_FILE=""
RESOLVE_LOG_REF=""
DOWNLOAD_LOG_REF=""
MATERIALIZE_LOG_REF=""
ANNOTATE_LOG_REF=""
ANNOTATION_MODE=""
AUDIT_TRAIL_REF=""
INDEX_REF=""
ROADMAP_REF=""
REVIEWER="Project Team"
RESULT="TBD"
SUMMARY="TBD"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --review-id) REVIEW_ID="$2"; shift 2 ;;
    --date) DATE_VALUE="$2"; shift 2 ;;
    --report-ref) REPORT_REF="$2"; shift 2 ;;
    --runtime-report-id) RUNTIME_REPORT_ID="$2"; shift 2 ;;
    --commit-id) COMMIT_ID="$2"; shift 2 ;;
    --trigger-ref) TRIGGER_REF="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --source-type) SOURCE_TYPE="$2"; shift 2 ;;
    --source-run-id) SOURCE_RUN_ID="$2"; shift 2 ;;
    --materialized-file) MATERIALIZED_FILE="$2"; shift 2 ;;
    --resolve-log-ref) RESOLVE_LOG_REF="$2"; shift 2 ;;
    --download-log-ref) DOWNLOAD_LOG_REF="$2"; shift 2 ;;
    --materialize-log-ref) MATERIALIZE_LOG_REF="$2"; shift 2 ;;
    --annotate-log-ref) ANNOTATE_LOG_REF="$2"; shift 2 ;;
    --annotation-mode) ANNOTATION_MODE="$2"; shift 2 ;;
    --audit-trail-ref) AUDIT_TRAIL_REF="$2"; shift 2 ;;
    --index-ref) INDEX_REF="$2"; shift 2 ;;
    --roadmap-ref) ROADMAP_REF="$2"; shift 2 ;;
    --reviewer) REVIEWER="$2"; shift 2 ;;
    --result) RESULT="$2"; shift 2 ;;
    --summary) SUMMARY="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--review-id" "$REVIEW_ID"
require_value "--date" "$DATE_VALUE"
require_value "--report-ref" "$REPORT_REF"
require_value "--runtime-report-id" "$RUNTIME_REPORT_ID"
require_value "--commit-id" "$COMMIT_ID"
require_value "--trigger-ref" "$TRIGGER_REF"
require_value "--run-id" "$RUN_ID"
require_value "--source-type" "$SOURCE_TYPE"
require_value "--source-run-id" "$SOURCE_RUN_ID"
require_value "--materialized-file" "$MATERIALIZED_FILE"
require_value "--resolve-log-ref" "$RESOLVE_LOG_REF"
require_value "--download-log-ref" "$DOWNLOAD_LOG_REF"
require_value "--materialize-log-ref" "$MATERIALIZE_LOG_REF"
require_value "--annotate-log-ref" "$ANNOTATE_LOG_REF"
require_value "--annotation-mode" "$ANNOTATION_MODE"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"
require_value "--index-ref" "$INDEX_REF"
require_value "--roadmap-ref" "$ROADMAP_REF"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
# Review Record: Baseline Fetch Success Runtime Review

- Review ID: \`$REVIEW_ID\`
- Date: \`$DATE_VALUE\`
- Scope: \`baseline fetch success runtime evidence\`
- Reviewer: \`$REVIEWER\`
- Status: \`Draft\`

## Reviewed Inputs

- \`$REPORT_REF\`
- actual runtime report converted from \`$RUNTIME_REPORT_ID\`
- \`.github/workflows/sil4-ci.yml\`
- baseline persistence and annotation policy artifacts

## Review Focus

- \`EVS-001\` report의 runtime field가 실제 workflow run과 일치하는지 검토한다.
- baseline source resolution, artifact materialization, annotation mode, log reference가 evidence chain에 충분히 연결되는지 점검한다.

## Required Checks

1. workflow run id \`$RUN_ID\`, trigger ref \`$TRIGGER_REF\`, commit id \`$COMMIT_ID\` 일치
2. baseline source type \`$SOURCE_TYPE\`, source run id \`$SOURCE_RUN_ID\`, materialized file \`$MATERIALIZED_FILE\` 일치
3. \`Resolve Baseline Artifact\` \`$RESOLVE_LOG_REF\`, \`Download Baseline Artifact\` \`$DOWNLOAD_LOG_REF\`, \`Materialize Baseline Summary\` \`$MATERIALIZE_LOG_REF\`, \`Render PR Annotation\` \`$ANNOTATE_LOG_REF\` log reference 존재
4. annotation mode \`$ANNOTATION_MODE\`가 expected policy와 일치
5. evidence index \`$INDEX_REF\`, audit trail \`$AUDIT_TRAIL_REF\`, roadmap linkage \`$ROADMAP_REF\`가 업데이트됨

## Decision

- Result: \`$RESULT\`
- Summary: \`$SUMMARY\`
EOF

echo "Rendered baseline fetch runtime review: $OUTPUT"
