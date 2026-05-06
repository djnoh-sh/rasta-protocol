#!/usr/bin/env bash
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-/tmp/rsrx-stack-memory-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-stack-memory-logs}"

CONFIGURE_LOG="$LOG_DIR/configure.log"
BUILD_LOG="$LOG_DIR/build.log"
SUMMARY_MD="$LOG_DIR/summary.md"
SUMMARY_ENV="$LOG_DIR/summary.env"
STACK_LIST="$LOG_DIR/stack_usage_files.txt"
RAW_STACK_LIST="$LOG_DIR/raw_stack_usage_files.txt"
STACK_ARTIFACT_DIR="$LOG_DIR/stack_usage"
MAP_FILE="$BUILD_DIR/rsrx_session_supervisor_flow_test.map"
MAP_ARTIFACT="$LOG_DIR/rsrx_session_supervisor_flow_test.map"

mkdir -p "$BUILD_DIR" "$LOG_DIR" "$STACK_ARTIFACT_DIR"

echo "[1/2] Configure stack/memory evidence build"
cmake \
  -S "$ROOT_DIR" \
  -B "$BUILD_DIR" \
  -DRSRX_ENABLE_STACK_MEMORY_EVIDENCE=ON \
  >"$CONFIGURE_LOG" 2>&1

echo "[2/2] Build stack/memory evidence target"
cmake \
  --build "$BUILD_DIR" \
  --target rsrx_session_supervisor_flow_test \
  -j4 \
  >"$BUILD_LOG" 2>&1

GIT_COMMIT="$(git -C "$ROOT_DIR" rev-parse HEAD 2>/dev/null || echo unavailable)"
C_COMPILER="$(grep '^CMAKE_C_COMPILER:FILEPATH=' "$BUILD_DIR/CMakeCache.txt" | sed 's/^CMAKE_C_COMPILER:FILEPATH=//')"
C_COMPILER_VERSION="$("$C_COMPILER" --version | sed -n '1p')"

find "$BUILD_DIR" -name '*.su' -type f | sort >"$RAW_STACK_LIST"

STACK_USAGE_COUNT="$(wc -l <"$RAW_STACK_LIST" | tr -d ' ')"
if [ "$STACK_USAGE_COUNT" = "0" ]; then
  echo "No stack-usage files were generated" >&2
  exit 1
fi

if [ ! -s "$MAP_FILE" ]; then
  echo "Representative linker map was not generated: $MAP_FILE" >&2
  exit 1
fi

rm -f "$STACK_ARTIFACT_DIR"/*.su "$STACK_LIST" "$MAP_ARTIFACT"
while IFS= read -r stack_file; do
  cp "$stack_file" "$STACK_ARTIFACT_DIR/$(basename "$stack_file")"
done <"$RAW_STACK_LIST"

find "$STACK_ARTIFACT_DIR" -name '*.su' -type f | sort >"$STACK_LIST"
cp "$MAP_FILE" "$MAP_ARTIFACT"
sed -i 's/[[:space:]]*$//' "$MAP_ARTIFACT"

cat >"$SUMMARY_MD" <<EOF2
# Stack/Memory Evidence Summary

| Item | Result |
| --- | --- |
| Configure | Pass |
| Commit | \`${GIT_COMMIT}\` |
| Compiler | \`$C_COMPILER\` |
| Compiler Version | \`${C_COMPILER_VERSION}\` |
| Evidence Option | \`RSRX_ENABLE_STACK_MEMORY_EVIDENCE=ON\` |
| Build Target | \`rsrx_session_supervisor_flow_test\` |
| Stack Usage File Count | ${STACK_USAGE_COUNT} |
| Stack Usage List | \`$STACK_LIST\` |
| Representative Linker Map | \`$MAP_ARTIFACT\` |
| Configure Log | \`$CONFIGURE_LOG\` |
| Build Log | \`$BUILD_LOG\` |

## Scope

This is a host-toolchain evidence run for the representative integration executable.
It does not replace target/vendor stack or memory evidence.
EOF2

cat >"$SUMMARY_ENV" <<EOF2
SUMMARY_MD=$SUMMARY_MD
CONFIGURE_LOG=$CONFIGURE_LOG
BUILD_LOG=$BUILD_LOG
GIT_COMMIT=$GIT_COMMIT
C_COMPILER=$C_COMPILER
C_COMPILER_VERSION=$C_COMPILER_VERSION
STACK_LIST=$STACK_LIST
RAW_STACK_LIST=$RAW_STACK_LIST
STACK_ARTIFACT_DIR=$STACK_ARTIFACT_DIR
STACK_USAGE_COUNT=$STACK_USAGE_COUNT
REPRESENTATIVE_LINKER_MAP=$MAP_ARTIFACT
EOF2

echo "Stack usage files: $STACK_USAGE_COUNT"
echo "Stack usage list: $STACK_LIST"
echo "Representative linker map: $MAP_ARTIFACT"
echo "Summary: $SUMMARY_MD"
