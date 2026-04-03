#!/usr/bin/env bash
set -eu

usage() {
  cat >&2 <<'EOF'
usage: render_operational_evidence_review_link_guidance.sh \
  --output <snippet-md> \
  --actual-close-guidance <guidance-md>
EOF
  exit 1
}

require_value() {
  local name="$1"
  local value="$2"
  if [ -z "$value" ]; then
    echo "missing required value: $name" >&2
    exit 1
  fi
}

read_guidance_value() {
  local file="$1"
  local key="$2"
  local line
  line="$(grep -m1 "^- ${key}: " "$file" || true)"
  if [ -z "$line" ]; then
    echo "missing required guidance line: $key" >&2
    exit 1
  fi
  line="${line#- ${key}: }"
  line="${line#\`}"
  line="${line%\`}"
  printf '%s\n' "$line"
}

OUTPUT=""
GUIDANCE=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --output) OUTPUT="$2"; shift 2 ;;
    --actual-close-guidance) GUIDANCE="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; usage ;;
  esac
done

require_value "--output" "$OUTPUT"
require_value "--actual-close-guidance" "$GUIDANCE"

if [ ! -f "$GUIDANCE" ]; then
  echo "actual close guidance file not found: $GUIDANCE" >&2
  exit 1
fi

TRACK="$(read_guidance_value "$GUIDANCE" "track")"
TRACKER_ROW_ARTIFACT="$(read_guidance_value "$GUIDANCE" "tracker row artifact")"
AUDIT_UPDATE_ARTIFACT="$(read_guidance_value "$GUIDANCE" "audit update artifact")"
PACKET_MANIFEST="$(read_guidance_value "$GUIDANCE" "packet manifest")"

mkdir -p "$(dirname "$OUTPUT")"

cat >"$OUTPUT" <<EOF
### Operational Evidence Review Link Guidance

- track: \`$TRACK\`
- tracker source: \`$TRACKER_ROW_ARTIFACT\`
- audit source: \`$AUDIT_UPDATE_ARTIFACT\`
- packet manifest: \`$PACKET_MANIFEST\`
- review target family: \`sil4/docs/reviews\`
- evidence index target: \`sil4/docs/evidence/evidence_index.md\`

Review/link policy:

- keep tracker and audit updates aligned with the generated packet manifest when preparing the next actual review entry
- use the evidence index target to register or cross-check the resulting actual evidence/review links after the close update is accepted
EOF

echo "Rendered operational evidence review link guidance: $OUTPUT"
