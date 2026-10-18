#!/usr/bin/env bash
# Same functional tests, for binaries built with -fsanitize=thread.
# Any ThreadSanitizer report makes the program exit with status 66.
cd "$(dirname "${BASH_SOURCE[0]}")/.." || exit 1
export TSAN_OPTIONS="halt_on_error=1:exitcode=66"
exec bash tests/smoke.sh
