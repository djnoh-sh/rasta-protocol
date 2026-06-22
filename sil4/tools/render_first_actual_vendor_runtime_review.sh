#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_first_actual_vendor_runtime_review.sh \
  --output <review-md> \
  --review-id <id> \
  --date <yyyy-mm-dd> \
  --report-ref <path-or-url> \
  --runtime-report-id <evidence-run-id> \
  --matrix-ref <path-or-url> \
  --tracking-ref <path-or-url> \
  --raw-evidence-ref <path-or-url> \
  --rule-id <rule-id> \
  --subset-id <subset> \
  --severity <severity> \
  --file-path <file> \
  --location <line-or-range> \
  --decision-ref <decision> \
  --audit-trail-ref <path-or-url> \
  --index-ref <path-or-url> \
  --roadmap-ref <path-or-url> \
  [--reviewer <name>] \
  [--result <Pass|Pass with Actions|Fail>] \
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
MATRIX_REF=""
TRACKING_REF=""
RAW_EVIDENCE_REF=""
RULE_ID=""
SUBSET_ID=""
SEVERITY=""
FILE_PATH=""
LOCATION=""
DECISION_REF=""
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
    --matrix-ref) MATRIX_REF="$2"; shift 2 ;;
    --tracking-ref) TRACKING_REF="$2"; shift 2 ;;
    --raw-evidence-ref) RAW_EVIDENCE_REF="$2"; shift 2 ;;
    --rule-id) RULE_ID="$2"; shift 2 ;;
    --subset-id) SUBSET_ID="$2"; shift 2 ;;
    --severity) SEVERITY="$2"; shift 2 ;;
    --file-path) FILE_PATH="$2"; shift 2 ;;
    --location) LOCATION="$2"; shift 2 ;;
    --decision-ref) DECISION_REF="$2"; shift 2 ;;
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
require_value "--matrix-ref" "$MATRIX_REF"
require_value "--tracking-ref" "$TRACKING_REF"
require_value "--raw-evidence-ref" "$RAW_EVIDENCE_REF"
require_value "--rule-id" "$RULE_ID"
require_value "--subset-id" "$SUBSET_ID"
require_value "--severity" "$SEVERITY"
require_value "--file-path" "$FILE_PATH"
require_value "--location" "$LOCATION"
require_value "--decision-ref" "$DECISION_REF"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"
require_value "--index-ref" "$INDEX_REF"
require_value "--roadmap-ref" "$ROADMAP_REF"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
# Review Record: First Actual Vendor Runtime Review

- Review ID: \`$REVIEW_ID\`
- Date: \`$DATE_VALUE\`
- Scope: \`first actual vendor runtime evidence\`
- Reviewer: \`$REVIEWER\`
- Status: \`Draft\`

## Reviewed Inputs

- \`$REPORT_REF\`
- actual runtime report converted from \`$RUNTIME_REPORT_ID\`
- \`$MATRIX_REF\`
- \`$TRACKING_REF\`

## Review Focus

- \`EVS-003\`~\`EVS-008\` evidence chain이 실제 vendor finding 값으로 일관되게 연결되는지 검토한다.
- raw evidence reference, classification, matrix entry, tracking link, audit trail update가 모두 operational state인지 점검한다.

## Required Checks

1. raw vendor output/source reference \`$RAW_EVIDENCE_REF\`가 역추적 가능함
2. vendor rule id \`$RULE_ID\`, subset \`$SUBSET_ID\`, severity \`$SEVERITY\`, file \`$FILE_PATH\`, location \`$LOCATION\`이 report와 matrix에서 일치
3. review decision과 tracking reference \`$DECISION_REF\`, \`$TRACKING_REF\`가 일치
4. actual matrix entry \`$MATRIX_REF\`와 audit trail reference \`$AUDIT_TRAIL_REF\`가 업데이트됨
5. evidence index \`$INDEX_REF\`와 roadmap linkage \`$ROADMAP_REF\`가 업데이트됨

## Decision

- Result: \`$RESULT\`
- Summary: \`$SUMMARY\`
EOF

echo "Rendered first actual vendor runtime review: $OUTPUT"
