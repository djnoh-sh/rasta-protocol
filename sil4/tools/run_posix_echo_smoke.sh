#!/usr/bin/env bash
set -eu

REPO_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
EXAMPLE_DIR="$REPO_DIR/sil4/examples/posix_echo"
BUILD_DIR="${BUILD_DIR:-/tmp/rsrx-ci-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-example-smoke-logs}"
VERIFICATION_LOG_DIR="${LOG_DIR}/verification"
EXAMPLE_BUILD_LOG="${LOG_DIR}/example_build.log"
SMOKE_RUN_LOG="${LOG_DIR}/smoke_run.log"
SUMMARY_MD="${LOG_DIR}/summary.md"

mkdir -p "$LOG_DIR"

echo "[example-smoke 1/3] Core verification"
BUILD_DIR="$BUILD_DIR" LOG_DIR="$VERIFICATION_LOG_DIR" \
	bash "$SCRIPT_DIR/run_ci_verification.sh"

echo "[example-smoke 2/3] Example build"
make -C "$EXAMPLE_DIR" BUILD_DIR="$BUILD_DIR" clean all >"$EXAMPLE_BUILD_LOG" 2>&1

echo "[example-smoke 3/3] Example smoke"
TMP_DIR="$LOG_DIR" BUILD_DIR="$BUILD_DIR" \
	bash "$EXAMPLE_DIR/run_smoke.sh" >"$SMOKE_RUN_LOG" 2>&1

cat >"$SUMMARY_MD" <<EOF
# SIL4 Example Smoke Summary

| Item | Result |
| --- | --- |
| Core Verification | Pass |
| Example Build | Pass |
| Example Smoke | Pass |

## Artifact Logs

- Core verification logs: \`${VERIFICATION_LOG_DIR}\`
- Example build log: \`${EXAMPLE_BUILD_LOG}\`
- Smoke run log: \`${SMOKE_RUN_LOG}\`
- Server log: \`${LOG_DIR}/server.log\`
- Client log: \`${LOG_DIR}/client.log\`
EOF

echo "Example build log: $EXAMPLE_BUILD_LOG"
echo "Smoke run log: $SMOKE_RUN_LOG"
echo "Summary: $SUMMARY_MD"
