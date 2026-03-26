#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: validate_operational_artifact_runner_receipt.sh --receipt <receipt-md>
EOF
  exit 1
}

require_line() {
  local pattern="$1"
  local file="$2"
  if ! grep -Eq "$pattern" "$file"; then
    echo "missing expected receipt line: $pattern" >&2
    exit 1
  fi
}

RECEIPT=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --receipt) RECEIPT="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

if [ -z "$RECEIPT" ]; then
  usage
fi

if [ ! -f "$RECEIPT" ]; then
  echo "receipt file not found: $RECEIPT" >&2
  exit 1
fi

require_line '^# Operational Artifact Runner Receipt$' "$RECEIPT"
require_line '^- Track: `(baseline|vendor)`$' "$RECEIPT"
require_line '^- Artifact Dir: `.+`$' "$RECEIPT"
require_line '^- Output Dir: `.+`$' "$RECEIPT"
require_line '^- Input Env: `.+`$' "$RECEIPT"
require_line '^- Packet Manifest: `.+`$' "$RECEIPT"
require_line '^- Summary Env: `.+`$' "$RECEIPT"
require_line '^## Next Open Targets$' "$RECEIPT"
require_line '^1\. open `.+/packet_manifest\.md`$' "$RECEIPT"
require_line '^2\. update tracker/audit trail using generated packet files$' "$RECEIPT"
require_line '^3\. retain actual artifact references linked from the generated packet$' "$RECEIPT"

echo "Operational artifact runner receipt validated: $RECEIPT"
