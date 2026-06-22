#!/usr/bin/env bash
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
APP="${SCRIPT_DIR}/rsrx_echo_app"
BUILD_DIR="${BUILD_DIR:-/tmp/sil4-build}"
TMP_DIR="${TMP_DIR:-/tmp/rsrx-posix-echo-failover-smoke}"
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

echo "[failover-smoke] starting server"
"${APP}" server \
	--redundant 1 \
	--duration-sec 12 \
	--primary-down-after-rx 3 \
	>"${SERVER_LOG}" 2>&1 &
SERVER_PID="$!"

sleep 1

echo "[failover-smoke] starting client"
"${APP}" client \
	--redundant 1 \
	--duration-sec 10 \
	--send-interval-ms 1000 \
	--message "hello during failover" \
	--primary-down-after-rx 3 \
	>"${CLIENT_LOG}" 2>&1

wait "${SERVER_PID}"
SERVER_PID=""

if ! grep -Eq "state=2->3|State change: 2 -> 3" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not reach ESTABLISHED" >&2
	exit 1
fi

if ! grep -q "Injected transport event: primary forced down after rx threshold" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not inject primary down" >&2
	exit 1
fi

if ! grep -q "failover=1" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not record failover" >&2
	exit 1
fi

if ! grep -q "Channel report: active=secondary" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not switch to secondary" >&2
	exit 1
fi

if ! grep -q "RX\\[4\\]:" "${SERVER_LOG}"; then
	echo "[failover-smoke] server did not receive post-failover data" >&2
	exit 1
fi

if ! grep -q "RX\\[4\\]:" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not receive post-failover echo" >&2
	exit 1
fi

if ! grep -q "Graceful disconnect status=0" "${CLIENT_LOG}"; then
	echo "[failover-smoke] client did not perform graceful disconnect" >&2
	exit 1
fi

echo "[failover-smoke] pass"
echo "[failover-smoke] server log: ${SERVER_LOG}"
echo "[failover-smoke] client log: ${CLIENT_LOG}"
