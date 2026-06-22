#!/usr/bin/env bash
set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: $0 <artifact-zip> <output-summary-env>" >&2
  exit 1
fi

ARTIFACT_ZIP="$1"
OUTPUT_SUMMARY_ENV="$2"

if [ ! -f "$ARTIFACT_ZIP" ]; then
  echo "artifact zip not found: $ARTIFACT_ZIP" >&2
  exit 1
fi

ENTRY_PATH="$(
  zipinfo -1 "$ARTIFACT_ZIP" \
    | grep -E '(^|/)summary\.env$' \
    | head -n 1 \
    || true
)"

if [ -z "$ENTRY_PATH" ]; then
  echo "summary.env entry not found in artifact: $ARTIFACT_ZIP" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT_SUMMARY_ENV")"
unzip -p "$ARTIFACT_ZIP" "$ENTRY_PATH" >"$OUTPUT_SUMMARY_ENV"
echo "Materialized baseline summary: $OUTPUT_SUMMARY_ENV"
