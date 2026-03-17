#!/usr/bin/env bash
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-/tmp/rsrx-ci-build}"
LOG_DIR="${LOG_DIR:-/tmp/rsrx-ci-logs}"

mkdir -p "$BUILD_DIR" "$LOG_DIR"

CONFIGURE_LOG="$LOG_DIR/configure.log"
BUILD_LOG="$LOG_DIR/build.log"
TEST_LOG="$LOG_DIR/tests.log"
CPPCHECK_LOG="$LOG_DIR/cppcheck.log"

echo "[1/4] Configure"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" >"$CONFIGURE_LOG" 2>&1

echo "[2/4] Build"
cmake --build "$BUILD_DIR" -j4 >"$BUILD_LOG" 2>&1

echo "[3/4] Run unit/integration executables"
{
  "$BUILD_DIR/rsrx_state_machine_test"
  "$BUILD_DIR/rsrx_orchestrator_test"
  "$BUILD_DIR/rsrx_platform_contract_test"
  "$BUILD_DIR/rsrx_platform_adapters_test"
  "$BUILD_DIR/rsrx_transport_contract_test"
  "$BUILD_DIR/rsrx_channel_manager_test"
  "$BUILD_DIR/rsrx_api_test"
  "$BUILD_DIR/rsrx_config_validator_test"
  "$BUILD_DIR/rsrx_codec_contract_test"
  "$BUILD_DIR/rsrx_codec_test"
  "$BUILD_DIR/rsrx_protocol_context_test"
  "$BUILD_DIR/rsrx_transport_supervisor_test"
  "$BUILD_DIR/rsrx_session_supervisor_flow_test"
} >"$TEST_LOG" 2>&1

echo "[4/4] Cppcheck"
cppcheck \
  --enable=warning,style,performance,portability \
  --std=c11 \
  --force \
  --inline-suppr \
  "$ROOT_DIR/include" \
  "$ROOT_DIR/src" \
  "$ROOT_DIR/tests/unit" \
  "$ROOT_DIR/tests/integration" \
  >"$CPPCHECK_LOG" 2>&1

echo "Configure log: $CONFIGURE_LOG"
echo "Build log: $BUILD_LOG"
echo "Test log: $TEST_LOG"
echo "Cppcheck log: $CPPCHECK_LOG"
