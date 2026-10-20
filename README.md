# codexion-extended

[![CI](https://github.com/imyassmine/codexion-extended/actions/workflows/ci.yml/badge.svg)](https://github.com/imyassmine/codexion-extended/actions/workflows/ci.yml)
![C](https://img.shields.io/badge/language-C-blue)
![POSIX](https://img.shields.io/badge/concurrency-pthreads%20%2B%20semaphores-lightgrey)

**The same concurrent simulation, implemented twice.** Codexion is a 42
concurrency project: coders compete for shared dongles around a hub. One
backend uses POSIX threads and mutexes, the other uses forked processes and
POSIX semaphores. Same CLI, same log format, same scheduling rules — a
side-by-side comparison of two concurrency models.

*Concurrency · synchronization · IPC · deadlock prevention · FIFO/EDF scheduling · C*

## Highlights

- **Deadlock-free by construction** — odd/even dongle acquisition order removes
  circular wait (one of the four Coffman conditions) in both backends.
- **Custom priority scheduler** — each dongle owns a hand-written binary
  min-heap implementing FIFO or EDF (earliest deadline first) arbitration, with
  a deterministic tie-break on coder id.
- **Thread backend (`codexion`)** — one pthread per coder, per-dongle mutex and
  condition variable, `pthread_cond_timedwait` so cooldown waiters wake exactly
  on time; a monitor thread polls deadlines every 500 µs for accurate burnout
  detection.
- **Process backend (`codexion_processes`)** — one `fork` per coder over
  `mmap(MAP_SHARED)` state, named POSIX semaphores replacing the mutex/condvar
  waits, a supervisor `waitpid`, sibling `SIGKILL` on burnout and full
  semaphore cleanup: no orphan process, no `/dev/shm` residue.
- **Validated under sanitizers** — ASan + UBSan and TSan builds; a Python log
  checker replays every run as a per-coder state machine to verify ordering,
  timestamps, burnout deadlines and completion counts.

## Two backends, one specification

| | `codexion` | `codexion_processes` |
| --- | --- | --- |
| Actors | one pthread per coder | one process per coder |
| Shared state | process memory | `mmap(MAP_SHARED \| MAP_ANONYMOUS)` |
| Mutual exclusion | mutex per dongle | named semaphore per dongle |
| Wake-up | condition variable + timed wait | grant semaphore + `sem_timedwait` |
| Shutdown | join all threads | supervisor `waitpid` + semaphore unlink |

## Quick start

```console
$ make
$ ./codexion 5 800 200 200 200 3 10 edf
$ ./codexion_processes 5 800 200 200 200 3 10 edf
```

Arguments: `coders time_to_burnout time_to_compile time_to_debug
time_to_refactor compiles_required dongle_cooldown <fifo|edf>`, all times in
milliseconds. A coder burns out as soon as `time_to_burnout` ms have elapsed
since its last compile started without a new compile beginning; the run stops
on the first burnout or once every coder has compiled `compiles_required`
times.

```text
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
400 1 is refactoring
```

## Testing

```console
$ make test            # 19 checks: feasible scenarios, invalid arguments, cleanup
$ make debug           # rebuild with -g3 -fsanitize=address,undefined
$ bash tests/tsan.sh   # after building with -fsanitize=thread
```

The test scripts need `python3` (log checker), GNU `timeout` and `pgrep`.

`tests/check_log.py` validates every run's log: per-coder state machine,
monotonic timestamps, single burnout within tolerance, no output after the
stop, and completion counts on successful runs. CI runs the suite in three
configurations — plain, ASan + UBSan and TSan
([workflow](.github/workflows/ci.yml)).

## Under the hood

- **Cooldown** — a released dongle gets `ready_time = now + cooldown`; timed
  waits wake exactly then, and the process backend reserves the next owner at
  release time.
- **Logging** — one lock/semaphore around all output, so lines never interleave
  and nothing is printed after the simulation stops.
- **Code size** — `coders/` (threads, 557 lines of C) and `coders_processes/`
  (processes + semaphores, 757 lines of C).

## Scope

Linux/POSIX only (pthreads, named semaphores, `mmap`). No UI: the simulation is
driven by arguments and validated from its log.

---

42 Paris curriculum project · solo · Yassmine Abdallah ([@imyassmine](https://github.com/imyassmine))
