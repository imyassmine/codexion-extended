#!/usr/bin/env python3
"""Validate a codexion log: per-coder state machine, timestamps, burnout, counts.

Usage: check_log.py LOG --coders N --burnout MS [--required K] [--expect complete|burnout]
"""

import argparse
import re
import sys

LINE = re.compile(r"^(\d+) (\d+) (has taken a dongle|is compiling|is debugging"
                  r"|is refactoring|burned out)$")
NEXT = {
    ("idle", "has taken a dongle"): "one",
    ("one", "has taken a dongle"): "two",
    ("two", "is compiling"): "compiling",
    ("compiling", "is debugging"): "debugging",
    ("debugging", "is refactoring"): "refactoring",
    ("refactoring", "has taken a dongle"): "one",
}
TOLERANCE = 150  # ms, generous so slow CI runners do not cause flaky failures


def check(lines, a):
    state = {c: "idle" for c in range(1, a.coders + 1)}
    compiles = dict.fromkeys(state, 0)
    last = dict.fromkeys(state, 0)
    burnout = None
    prev_ts = 0

    for n, raw in enumerate(lines, 1):
        m = LINE.match(raw)
        if not m:
            return "line %d: malformed %r" % (n, raw)
        ts, coder, msg = int(m[1]), int(m[2]), m[3]
        if ts < prev_ts:
            return "line %d: timestamp goes backwards" % n
        prev_ts = ts
        if coder not in state:
            return "line %d: unknown coder %d" % (n, coder)
        if burnout is not None:
            if ts > burnout + 5:
                return "line %d: output after the burnout" % n
            continue
        if msg == "burned out":
            if a.expect == "complete":
                return "line %d: unexpected burnout" % n
            if abs(ts - (last[coder] + a.burnout)) > TOLERANCE:
                return "line %d: burnout at %d, expected ~%d" % (
                    n, ts, last[coder] + a.burnout)
            burnout = ts
            continue
        if (state[coder], msg) not in NEXT:
            return "line %d: coder %d: %r after %r" % (n, coder, msg, state[coder])
        state[coder] = NEXT[(state[coder], msg)]
        if msg == "is compiling":
            if compiles[coder] and ts > last[coder] + a.burnout + TOLERANCE:
                return "line %d: coder %d missed its deadline" % (n, coder)
            compiles[coder] += 1
            last[coder] = ts

    if a.expect == "burnout" and burnout is None:
        return "expected a burnout, none found"
    if a.expect == "complete" and a.required:
        for c, k in compiles.items():
            if k < a.required:
                return "coder %d compiled %d/%d times" % (c, k, a.required)
    return None


def main():
    p = argparse.ArgumentParser()
    p.add_argument("log")
    p.add_argument("--coders", type=int, required=True)
    p.add_argument("--burnout", type=int, required=True)
    p.add_argument("--required", type=int, default=0)
    p.add_argument("--expect", choices=("complete", "burnout"), default="complete")
    a = p.parse_args()
    with open(a.log, encoding="utf-8") as f:
        lines = [x.rstrip("\n") for x in f if x.strip()]
    error = check(lines, a) if lines else "empty log"
    if error:
        print("   " + error)
    return 1 if error else 0


if __name__ == "__main__":
    sys.exit(main())
