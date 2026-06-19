#!/usr/bin/env bash
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
APP="${SCRIPT_DIR}/rsrx_echo_app"
BUILD_DIR="${BUILD_DIR:-/tmp/sil4-build}"
SERVER_PORT="${SERVER_PORT:-8888}"
CLIENT_PORT="${CLIENT_PORT:-8889}"
SERVER_DURATION_SEC="${SERVER_DURATION_SEC:-15}"
CLIENT_DURATION_SEC="${CLIENT_DURATION_SEC:-6}"
SEND_INTERVAL_MS="${SEND_INTERVAL_MS:-2000}"
MESSAGE="${MESSAGE:-hello from smoke}"
TMP_DIR="${TMP_DIR:-/tmp/rsrx-posix-echo-smoke}"
SERVER_LOG="${TMP_DIR}/server.log"
CLIENT_LOG="${TMP_DIR}/client.log"
SERVER_PID=""

cleanup() {
	if [ -n "${SERVER_PID}" ] && kill -0 "${SERVER_PID}" 2>/dev/null; then
		kill "${SERVER_PID}" 2>/dev/null || true
		wait "${SERVER_PID}" 2>/dev/null || true
	fi
}

mkdir -p "${TMP_DIR}"
rm -f "${SERVER_LOG}" "${CLIENT_LOG}"
trap cleanup EXIT INT TERM

if [ ! -x "${APP}" ]; then
	echo "missing executable: ${APP}" >&2
	echo "build first with: make -C sil4/examples/posix_echo BUILD_DIR=${BUILD_DIR}" >&2
	exit 1
fi

echo "[smoke] starting server"
"${APP}" server \
	--local-port "${SERVER_PORT}" \
	--remote-port "${CLIENT_PORT}" \
	--duration-sec "${SERVER_DURATION_SEC}" \
	>"${SERVER_LOG}" 2>&1 &
SERVER_PID="$!"

sleep 1

echo "[smoke] starting client"
"${APP}" client \
	--local-port "${CLIENT_PORT}" \
	--remote-port "${SERVER_PORT}" \
	--duration-sec "${CLIENT_DURATION_SEC}" \
	--send-interval-ms "${SEND_INTERVAL_MS}" \
	--message "${MESSAGE}" \
	>"${CLIENT_LOG}" 2>&1

wait "${SERVER_PID}"
SERVER_PID=""

if ! grep -Eq "State change: 2 -> 3|state=2->3" "${CLIENT_LOG}"; then
	echo "[smoke] client did not reach ESTABLISHED" >&2
	exit 1
fi

if ! grep -q "TX\\[1\\]:" "${CLIENT_LOG}"; then
	echo "[smoke] client did not send application data" >&2
	exit 1
fi

if ! grep -q "RX\\[1\\]:" "${SERVER_LOG}"; then
	echo "[smoke] server did not receive application data" >&2
	exit 1
fi

if ! grep -q "Echo send status=0" "${SERVER_LOG}"; then
	echo "[smoke] server did not echo successfully" >&2
	exit 1
fi

if ! grep -q "RX\\[1\\]:" "${CLIENT_LOG}"; then
	echo "[smoke] client did not receive echo" >&2
	exit 1
fi

if ! grep -q "Graceful disconnect status=0" "${CLIENT_LOG}"; then
	echo "[smoke] client did not perform graceful disconnect" >&2
	exit 1
fi

echo "[smoke] pass"
echo "[smoke] server log: ${SERVER_LOG}"
echo "[smoke] client log: ${CLIENT_LOG}"
