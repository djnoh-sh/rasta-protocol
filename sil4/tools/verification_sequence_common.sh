#!/usr/bin/env bash

phase_marker_path() {
  local log_dir="$1"
  local phase_name="$2"
  printf "%s/.phase_%s.ok" "$log_dir" "$phase_name"
}

reset_verification_phase_markers() {
  local log_dir="$1"
  rm -f \
    "$(phase_marker_path "$log_dir" configure)" \
    "$(phase_marker_path "$log_dir" build)" \
    "$(phase_marker_path "$log_dir" test)" \
    "$(phase_marker_path "$log_dir" cppcheck)"
}

mark_verification_phase_complete() {
  local log_dir="$1"
  local phase_name="$2"
  : >"$(phase_marker_path "$log_dir" "$phase_name")"
}

require_verification_phase_complete() {
  local log_dir="$1"
  local phase_name="$2"
  local marker
  marker="$(phase_marker_path "$log_dir" "$phase_name")"
  if [ ! -f "$marker" ]; then
    echo "verification phase prerequisite not satisfied: $phase_name" >&2
    exit 1
  fi
}

require_test_executable_ready() {
  local executable_path="$1"
  if [ ! -x "$executable_path" ]; then
    echo "required test executable is missing or not executable: $executable_path" >&2
    exit 1
  fi
}
