#!/usr/bin/env bash
# Functional tests for codexion and codexion_processes.
# Each run is validated by tests/check_log.py; `timeout` prevents hangs.
# Usage: bash tests/smoke.sh   (after `make`)

cd "$(dirname "${BASH_SOURCE[0]}")/.." || exit 1
# Linux truncates process names to 15 characters: "codexion_processes" -> "codexion_proces"
PROC_COMM=codexion_proces
tmp="$(mktemp -d)"
fail=0
trap 'rm -rf "$tmp"; pkill -9 -x "$PROC_COMM" 2>/dev/null; rm -f /dev/shm/sem.codexion_b_*' EXIT

report() { # report <label> <status>
	if [ "$2" -eq 0 ]; then echo "ok   $1"; else echo "FAIL $1"; fail=1; fi
}

# simulate <bin> <coders> <burnout> <required> <expect> <args...>
simulate() {
	local bin="$1" coders="$2" burnout="$3" required="$4" expect="$5"
	shift 5
	timeout 20 "./$bin" "$@" >"$tmp/log" 2>"$tmp/err"
	local rc=$?
	if [ "$rc" -eq 0 ]; then
		python3 tests/check_log.py "$tmp/log" --coders "$coders" \
			--burnout "$burnout" --required "$required" --expect "$expect"
		rc=$?
	else
		head -n 5 "$tmp/err"
	fi
	report "$bin $*" "$rc"
}

for bin in codexion codexion_processes; do
	[ -x "./$bin" ] || { echo "$bin is missing: run make first"; exit 1; }
	#         coders burnout required expect    args
	simulate "$bin" 4 3000 2 complete 4 3000 100 50 50 2 10 fifo
	simulate "$bin" 5 3000 2 complete 5 3000 100 50 50 2 20 edf
	simulate "$bin" 1 400  1 burnout  1 400  100 100 100 1 0 fifo

	for bad in "" "4 800 200 200 200 3 10" "4 -800 200 200 200 3 10 fifo" \
		"4 800 200 x 200 3 10 fifo" "4 800 200 200 200 3 10 lifo" \
		"0 800 200 200 200 3 10 fifo"; do
		# shellcheck disable=SC2086
		timeout 5 "./$bin" $bad >/dev/null 2>&1
		rc=$?
		[ "$rc" -ne 0 ] && [ "$rc" -ne 124 ]
		report "$bin rejects: '$bad'" $?
	done
done

sleep 0.2
! ls /dev/shm | grep -q codexion_b_ && ! pgrep -x "$PROC_COMM" >/dev/null
report "bonus leaves no semaphore or process behind" $?

exit "$fail"
