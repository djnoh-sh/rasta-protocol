#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_first_actual_vendor_evidence.sh \
  --output <report-md> \
  --report-id <id> \
  --execution-date <yyyy-mm-dd> \
  --tool-source <tool-name> \
  --commit-id <commit> \
  --ref-name <pr-or-branch> \
  --run-id <workflow-or-capture-id> \
  --job-name <job-or-capture-name> \
  --trigger-ref <git-ref> \
  --artifact-name <artifact-name> \
  --tool-version <version> \
  --raw-evidence-type <type> \
  --raw-evidence-location <location> \
  --export-format <format> \
  --capture-timestamp <timestamp> \
  --reviewer-access-path <path> \
  --vendor-rule-id <rule-id> \
  --vendor-rule-family <family> \
  --subset-id <subset> \
  --severity <severity> \
  --file-path <file> \
  --location <line-or-range> \
  --initial-decision <decision> \
  --workflow-url <url> \
  --raw-artifact-ref <url-or-ref> \
  --vendor-report-ref <path-or-url> \
  --vendor-review-ref <path-or-url> \
  --vendor-matrix-ref <path-or-url> \
  --tracking-ref <path-or-url> \
  --audit-trail-ref <path-or-url> \
  [--event-type <event>]
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
REPORT_ID=""
EXECUTION_DATE=""
TOOL_SOURCE=""
COMMIT_ID=""
REF_NAME=""
RUN_ID=""
JOB_NAME=""
TRIGGER_REF=""
ARTIFACT_NAME=""
TOOL_VERSION=""
RAW_EVIDENCE_TYPE=""
RAW_EVIDENCE_LOCATION=""
EXPORT_FORMAT=""
CAPTURE_TIMESTAMP=""
REVIEWER_ACCESS_PATH=""
VENDOR_RULE_ID=""
VENDOR_RULE_FAMILY=""
SUBSET_ID=""
SEVERITY=""
FILE_PATH=""
LOCATION=""
INITIAL_DECISION=""
WORKFLOW_URL=""
RAW_ARTIFACT_REF=""
VENDOR_REPORT_REF=""
VENDOR_REVIEW_REF=""
VENDOR_MATRIX_REF=""
TRACKING_REF=""
AUDIT_TRAIL_REF=""
EVENT_TYPE="vendor_capture"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --report-id) REPORT_ID="$2"; shift 2 ;;
    --execution-date) EXECUTION_DATE="$2"; shift 2 ;;
    --tool-source) TOOL_SOURCE="$2"; shift 2 ;;
    --commit-id) COMMIT_ID="$2"; shift 2 ;;
    --ref-name) REF_NAME="$2"; shift 2 ;;
    --run-id) RUN_ID="$2"; shift 2 ;;
    --job-name) JOB_NAME="$2"; shift 2 ;;
    --trigger-ref) TRIGGER_REF="$2"; shift 2 ;;
    --artifact-name) ARTIFACT_NAME="$2"; shift 2 ;;
    --tool-version) TOOL_VERSION="$2"; shift 2 ;;
    --raw-evidence-type) RAW_EVIDENCE_TYPE="$2"; shift 2 ;;
    --raw-evidence-location) RAW_EVIDENCE_LOCATION="$2"; shift 2 ;;
    --export-format) EXPORT_FORMAT="$2"; shift 2 ;;
    --capture-timestamp) CAPTURE_TIMESTAMP="$2"; shift 2 ;;
    --reviewer-access-path) REVIEWER_ACCESS_PATH="$2"; shift 2 ;;
    --vendor-rule-id) VENDOR_RULE_ID="$2"; shift 2 ;;
    --vendor-rule-family) VENDOR_RULE_FAMILY="$2"; shift 2 ;;
    --subset-id) SUBSET_ID="$2"; shift 2 ;;
    --severity) SEVERITY="$2"; shift 2 ;;
    --file-path) FILE_PATH="$2"; shift 2 ;;
    --location) LOCATION="$2"; shift 2 ;;
    --initial-decision) INITIAL_DECISION="$2"; shift 2 ;;
    --workflow-url) WORKFLOW_URL="$2"; shift 2 ;;
    --raw-artifact-ref) RAW_ARTIFACT_REF="$2"; shift 2 ;;
    --vendor-report-ref) VENDOR_REPORT_REF="$2"; shift 2 ;;
    --vendor-review-ref) VENDOR_REVIEW_REF="$2"; shift 2 ;;
    --vendor-matrix-ref) VENDOR_MATRIX_REF="$2"; shift 2 ;;
    --tracking-ref) TRACKING_REF="$2"; shift 2 ;;
    --audit-trail-ref) AUDIT_TRAIL_REF="$2"; shift 2 ;;
    --event-type) EVENT_TYPE="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--report-id" "$REPORT_ID"
require_value "--execution-date" "$EXECUTION_DATE"
require_value "--tool-source" "$TOOL_SOURCE"
require_value "--commit-id" "$COMMIT_ID"
require_value "--ref-name" "$REF_NAME"
require_value "--run-id" "$RUN_ID"
require_value "--job-name" "$JOB_NAME"
require_value "--trigger-ref" "$TRIGGER_REF"
require_value "--artifact-name" "$ARTIFACT_NAME"
require_value "--tool-version" "$TOOL_VERSION"
require_value "--raw-evidence-type" "$RAW_EVIDENCE_TYPE"
require_value "--raw-evidence-location" "$RAW_EVIDENCE_LOCATION"
require_value "--export-format" "$EXPORT_FORMAT"
require_value "--capture-timestamp" "$CAPTURE_TIMESTAMP"
require_value "--reviewer-access-path" "$REVIEWER_ACCESS_PATH"
require_value "--vendor-rule-id" "$VENDOR_RULE_ID"
require_value "--vendor-rule-family" "$VENDOR_RULE_FAMILY"
require_value "--subset-id" "$SUBSET_ID"
require_value "--severity" "$SEVERITY"
require_value "--file-path" "$FILE_PATH"
require_value "--location" "$LOCATION"
require_value "--initial-decision" "$INITIAL_DECISION"
require_value "--workflow-url" "$WORKFLOW_URL"
require_value "--raw-artifact-ref" "$RAW_ARTIFACT_REF"
require_value "--vendor-report-ref" "$VENDOR_REPORT_REF"
require_value "--vendor-review-ref" "$VENDOR_REVIEW_REF"
require_value "--vendor-matrix-ref" "$VENDOR_MATRIX_REF"
require_value "--tracking-ref" "$TRACKING_REF"
require_value "--audit-trail-ref" "$AUDIT_TRAIL_REF"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
# First Actual Vendor Finding Report

## Document Control

- Report ID: \`$REPORT_ID\`
- Status: \`Draft\`
- Execution Date: \`$EXECUTION_DATE\`
- Tool Source: \`$TOOL_SOURCE\`
- Commit ID: \`$COMMIT_ID\`
- PR or Branch: \`$REF_NAME\`

## Purpose

이 문서는 \`EVS-003\`~\`EVS-008\`을 실제 first vendor finding 결과로 채우기 위한 runtime evidence report다.

## Runtime Context

| Field | Value |
| --- | --- |
| Event Type | \`$EVENT_TYPE\` |
| Workflow Run ID | \`$RUN_ID\` |
| Job Name | \`$JOB_NAME\` |
| Trigger Ref | \`$TRIGGER_REF\` |
| Artifact Name | \`$ARTIFACT_NAME\` |
| Tool Version | \`$TOOL_VERSION\` |

## Raw Evidence Reference

| Field | Value |
| --- | --- |
| Raw Evidence Type | \`$RAW_EVIDENCE_TYPE\` |
| Raw Evidence Location | \`$RAW_EVIDENCE_LOCATION\` |
| Export Format | \`$EXPORT_FORMAT\` |
| Capture Timestamp | \`$CAPTURE_TIMESTAMP\` |
| Reviewer Access Path | \`$REVIEWER_ACCESS_PATH\` |

## First Finding Classification

| Field | Value |
| --- | --- |
| Vendor Rule ID | \`$VENDOR_RULE_ID\` |
| Vendor Rule Family | \`$VENDOR_RULE_FAMILY\` |
| Subset ID | \`$SUBSET_ID\` |
| Severity | \`$SEVERITY\` |
| File | \`$FILE_PATH\` |
| Location | \`$LOCATION\` |
| Initial Decision | \`$INITIAL_DECISION\` |

## Required Downstream Artifacts

| Deliverable | Target Artifact | Status |
| --- | --- | --- |
| Vendor finding report | \`$VENDOR_REPORT_REF\` | \`Prepared\` |
| Vendor finding review | \`$VENDOR_REVIEW_REF\` | \`Prepared\` |
| Vendor matrix actual entry | \`$VENDOR_MATRIX_REF\` | \`Prepared\` |
| Deviation or fix tracking | \`$TRACKING_REF\` | \`Prepared\` |
| Audit trail update | \`$AUDIT_TRAIL_REF\` | \`Prepared\` |

## Required Linkage

- workflow URL: \`$WORKFLOW_URL\`
- raw artifact URL or secured reference: \`$RAW_ARTIFACT_REF\`
- vendor report artifact: \`$VENDOR_REPORT_REF\`
- vendor review artifact: \`$VENDOR_REVIEW_REF\`
- vendor matrix actual entry reference: \`$VENDOR_MATRIX_REF\`
- deviation or fix tracking reference: \`$TRACKING_REF\`
- audit trail reference: \`$AUDIT_TRAIL_REF\`

## Expected Confirmation Checklist

- [ ] raw vendor output/source reference가 역추적 가능함
- [ ] first vendor finding report가 subset/severity/rule-id 기준으로 채워짐
- [ ] review record가 classification/action 결정을 검토함
- [ ] actual vendor matrix entry가 sample이 아니라 운영값으로 채워짐
- [ ] deviation 또는 fix-required linkage가 명시됨
- [ ] audit trail closeout에 actual vendor evidence가 연결됨

## Conclusion

- Vendor Evidence Status: \`TBD\`
- Follow-up Review: \`RV-TBD\`
EOF

echo "Rendered first actual vendor evidence: $OUTPUT"
