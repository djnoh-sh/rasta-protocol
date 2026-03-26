#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_audit_evidence_update.sh \
  --output <snippet-md> \
  --track <baseline|vendor> \
  --date <yyyy-mm-dd> \
  --report-ref <path-or-url> \
  --review-ref <path-or-url> \
  --tracker-ref <path-or-url> \
  [--matrix-ref <path-or-url>] \
  [--tracking-ref <path-or-url>]
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
TRACK=""
DATE_VALUE=""
REPORT_REF=""
REVIEW_REF=""
TRACKER_REF=""
MATRIX_REF=""
TRACKING_REF=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --track) TRACK="$2"; shift 2 ;;
    --date) DATE_VALUE="$2"; shift 2 ;;
    --report-ref) REPORT_REF="$2"; shift 2 ;;
    --review-ref) REVIEW_REF="$2"; shift 2 ;;
    --tracker-ref) TRACKER_REF="$2"; shift 2 ;;
    --matrix-ref) MATRIX_REF="$2"; shift 2 ;;
    --tracking-ref) TRACKING_REF="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--track" "$TRACK"
require_value "--date" "$DATE_VALUE"
require_value "--report-ref" "$REPORT_REF"
require_value "--review-ref" "$REVIEW_REF"
require_value "--tracker-ref" "$TRACKER_REF"

mkdir -p "$(dirname "$OUTPUT")"

case "$TRACK" in
  baseline)
    cat >"$OUTPUT" <<EOF
### Baseline Fetch Actual Evidence Update ($DATE_VALUE)

- runtime report: \`$REPORT_REF\`
- runtime review: \`$REVIEW_REF\`
- execution tracker: \`$TRACKER_REF\`
EOF
    ;;
  vendor)
    require_value "--matrix-ref" "$MATRIX_REF"
    require_value "--tracking-ref" "$TRACKING_REF"
    cat >"$OUTPUT" <<EOF
### Vendor Finding Actual Evidence Update ($DATE_VALUE)

- runtime report: \`$REPORT_REF\`
- runtime review: \`$REVIEW_REF\`
- actual matrix entry: \`$MATRIX_REF\`
- deviation or fix tracking: \`$TRACKING_REF\`
- execution tracker: \`$TRACKER_REF\`
EOF
    ;;
  *)
    echo "unsupported track: $TRACK" >&2
    usage
    ;;
esac

echo "Rendered audit evidence update: $OUTPUT"
