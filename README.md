*This activity has been created as part of the 42 curriculum by yassabda.*

# Codexion — Threads and Processes

## Description

**Codexion** is a concurrency simulation inspired by the dining philosophers
problem. A number of coders sit around a circular co-working hub, and each of
them alternates between **compiling**, **debugging** and **refactoring**.
Compiling quantum code requires **two USB dongles held simultaneously**, and
there are exactly as many dongles as coders, so neighbours have to share them.
A coder must start compiling regularly: if they wait longer than
`time_to_burnout` milliseconds since their last compile started, they **burn
out** and the simulation stops.

This repository merges two 42 projects into a single code base:

* **[CODEXION](https://github.com/imyassmine/CODEXION)** — the mandatory
  simulation, written with POSIX **threads, mutexes and condition variables**,
  including the FIFO/EDF dongle scheduler and its heap.
* **[PHILOSOPHERS](https://github.com/imyassmine/PHILOSOPHERS)** — in particular
  its `philo_bonus`: the same class of problem solved with **forked processes
  and POSIX semaphores**.

The merge keeps the Codexion code and semantics as the reference and adds a
second executable, `codexion_bonus`, which runs the **very same simulation**
(identical arguments, identical log format, identical scheduling rules) with a
**process + semaphore** backend. The mandatory `codexion` implementation is kept
as-is apart from a few small fixes described in
[Changes made while merging](#changes-made-while-merging).

| Executable       | Actors              | Shared state                       | Arbitration                     |
|------------------|---------------------|------------------------------------|---------------------------------|
| `codexion`       | one thread per coder | mutexes + condition variables     | per-dongle mutex + heap + cond  |
| `codexion_bonus` | one process per coder | shared memory (`mmap`) + named POSIX semaphores | per-dongle semaphore + heap + grant semaphores |

Both executables accept the exact same command line and print the exact same
log lines, which makes it easy to compare the two backends.

## Instructions

### Requirements

* Linux, a C compiler (`cc`, e.g. gcc or clang) and GNU `make`;
* `python3` to run the functional test suite (the checker is a standard-library
  script, no dependency);
* optional: `norminette` for `make norm`.

### Compilation

```bash
make        # build both executables (codexion and codexion_bonus)
make bonus  # build only codexion_bonus
make debug  # rebuild both with -g3 and -fsanitize=address,undefined
make clean  # remove object files
make fclean # remove object files and executables
make re     # rebuild from scratch
make test   # build, then run tests/smoke.sh
make norm   # run norminette if it is installed
make help   # show the available targets
```

The Makefile uses `cc -Wall -Wextra -Werror -pthread` and does not relink an
already up-to-date binary.

### Usage

Both executables take the eight mandatory Codexion arguments:

```bash
./codexion       number_of_coders time_to_burnout time_to_compile time_to_debug \
                 time_to_refactor number_of_compiles_required dongle_cooldown scheduler
./codexion_bonus number_of_coders time_to_burnout time_to_compile time_to_debug \
                 time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument                      | Description                                                                 |
|-------------------------------|-----------------------------------------------------------------------------|
| `number_of_coders`            | Number of coders, and therefore number of dongles                           |
| `time_to_burnout`             | Maximum delay (ms) since the last compile start before a burnout            |
| `time_to_compile`             | Time (ms) spent compiling while holding two dongles                        |
| `time_to_debug`               | Time (ms) spent debugging                                                   |
| `time_to_refactor`            | Time (ms) spent refactoring                                                 |
| `number_of_compiles_required` | The simulation stops once every coder has reached this many compilations    |
| `dongle_cooldown`             | Time (ms) a dongle stays unavailable after being released                   |
| `scheduler`                   | `fifo` (first in, first out) or `edf` (earliest deadline first)             |

Example:

```bash
./codexion       5 800 200 200 200 3 10 edf
./codexion_bonus 5 800 200 200 200 3 10 edf
```

Every state change is printed on a single, non-interleaved line:

```text
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
400 1 is refactoring
...
1505 4 burned out
```

The timestamp is the number of milliseconds since the beginning of the
simulation. The simulation stops either when a coder burns out or when every
coder has compiled at least `number_of_compiles_required` times.

### Tests

```bash
make test   # functional suite: both binaries, 26 checks
bash tests/smoke.sh
bash tests/tsan.sh   # after a -fsanitize=thread build
```

* `tests/check_log.py` replays a log as a per-coder state machine
  (`idle → dongle → dongle → compiling → debugging → refactoring → idle`),
  checks the timestamp monotonicity, the coder ids, the absence of interleaved
  or malformed lines, that a burnout is reported once and within tolerance of
  its actual deadline, that no coder silently misses its deadline, and — for
  successful runs — that every coder reached the required number of
  compilations.
* `tests/smoke.sh` runs feasible scenarios (`fifo`, `edf`, odd/even numbers of
  coders, cooldown, single coder), rejects invalid arguments, and checks that
  the bonus backend leaves **no named semaphore and no orphan process** behind.
* `tests/tsan.sh` runs the ThreadSanitizer build on a few scenarios and fails
  on any sanitizer report.

## Project layout

```text
.
├── Makefile
├── README.md
├── .github/workflows/ci.yml
├── coders/                      # mandatory backend: threads + mutexes
│   ├── include/codexion.h
│   └── src/
│       ├── main.c               parsing.c       initialization.c
│       ├── heap.c               dongle.c        coder.c
│       ├── monitor.c            threads.c       safe_access.c
│       ├── utils.c              cleanup.c
├── coders_bonus/                # bonus backend: processes + semaphores
│   ├── include/codexion_bonus.h
│   └── src/
│       ├── main_bonus.c         parsing_bonus.c   initialization_bonus.c
│       ├── shared_memory_bonus.c  semaphores_bonus.c
│       ├── cleanup_semaphores_bonus.c
│       ├── heap_bonus.c         dongle_bonus.c    dongle_acquire_bonus.c
│       ├── coder_bonus.c        monitor_bonus.c   processes_bonus.c
│       ├── safe_access_bonus.c  utils_bonus.c     cleanup_bonus.c
└── tests/
    ├── check_log.py
    ├── smoke.sh
    └── tsan.sh
```

## Architecture and technical choices

### Shared simulation rules (both backends)

* **Two dongles per compile.** A coder takes its left and its right dongle
  before compiling and puts both back afterwards.
* **Deadlock-free acquisition order.** Odd-numbered coders take their
  right-side dongle first and even-numbered coders take their left-side dongle
  first (the classic odd/even dining-philosophers ordering). This breaks the
  circular wait, so no coder can ever wait forever for a dongle held by a
  neighbour who is itself waiting.
* **Per-dongle priority queue.** Each dongle owns an explicit binary min-heap
  (implemented by hand, no standard library priority queue) storing the pending
  requests:
  * `fifo`: the key is a monotonically increasing arrival ticket, so requests
    are served in arrival order;
  * `edf`: the key is the coder's current burnout deadline
    (`last_compile_start + time_to_burnout`), so the coder closest to burning
    out is served first.
  Ties on the key are broken by coder id, which makes the policy fully
  deterministic.
* **Cooldown.** After a release, a dongle records
  `ready_time = now + dongle_cooldown` and cannot be granted before it. The
  thread backend lets waiters wake up exactly at `ready_time` with a timed
  condition wait; the process backend additionally reserves the next owner when
  the dongle is released and hands it over at `ready_time`. In both cases the
  releasing coder immediately goes back to debugging and is never blocked by
  the cooldown.
* **Precise burnout detection.** A dedicated monitor polls the coders'
  deadlines every 500 µs, so the `burned out` line is printed within the 10 ms
  tolerance required by the subject.
* **Serialized logging.** Every line is printed while holding one dedicated
  log lock/semaphore, so two messages can never be mixed on the same line, and
  nothing is printed once the simulation is stopping.
* **Stop conditions.** The simulation stops when a coder burns out or when
  every coder reached the required number of compilations.

### Mandatory backend — threads, mutexes, condition variables

Each coder is a `pthread_t`; a separate monitor thread watches the deadlines.
Each dongle has its own `pthread_mutex_t` protecting its state and heap, plus a
`pthread_cond_t` used to wake waiters. A coder registers itself in the dongle's
heap, then waits on the condition variable until it is both the scheduler's
choice **and** the cooldown has elapsed. The wait uses
`pthread_cond_timedwait` with the dongle's `ready_time` as the absolute
deadline, so a waiter wakes up by itself when the cooldown expires even if no
other thread broadcasts at that moment. Shared counters and flags are read and
written through small helpers that take the matching mutex.

### Bonus backend — processes, shared memory and semaphores

The simulation state is mapped in `mmap(MAP_SHARED | MAP_ANONYMOUS)` memory
before the `fork()` calls, so every child sees the same dongles, heaps, pid
table and stop flag (the mapping has the same address in every process, which
also makes the heap pointers valid everywhere). The parent forks one process per
coder and acts as a supervisor: it records the pid of every child in shared
memory, opens a start gate, then `waitpid`s for all of them.

Synchronisation uses **named POSIX semaphores** (`sem_open`, unique names built
from the parent pid so concurrent runs never collide):

* one **mutex semaphore per dongle**, protecting that dongle's shared state and
  heap — this replaces the per-dongle `pthread_mutex_t` of the thread backend;
* one **grant semaphore per coder**, posted by a releasing coder to hand the
  dongle to the next scheduled coder — this replaces the condition variable;
* a **log semaphore**, a **FIFO-ticket semaphore**, a **start-gate semaphore**
  and a **deadline semaphore** for the remaining shared values.

A coder waiting for a dongle registers once in the dongle's heap, then uses
`sem_timedwait` on its grant semaphore with the dongle's `ready_time` (or a
100 ms safety step) as deadline. This mirrors the timed condition wait of the
mandatory backend: cooldown expiry, scheduler grants and the stop flag are all
handled without any possibility of an indefinite wait. The releasing coder
reserves the dongle for the heap's top request and posts that coder's grant
semaphore, so FIFO/EDF ordering is preserved across processes.

Each child process creates its own **monitor thread**, which detects its own
burnout, prints the `burned out` line (serialized through the log semaphore,
only once), and sends `SIGKILL` to its sibling processes. The parent reaps every
child, so no zombie or orphan process is left behind; the parent then closes and
unlinks every named semaphore and unmaps the shared memory, so nothing is left
in `/dev/shm` either.

A process must never terminate while one of its threads still holds a shared
semaphore, because POSIX semaphores are not released when their holder dies.
The coder thread therefore never stops with `_exit` directly: when it is done it
sets a `full` flag, **joins its monitor thread**, and only then exits. The
monitor checks the flag before reading the deadline and again before reporting a
burnout, so an exiting coder can neither leave the deadline semaphore locked nor
be wrongly reported as burned out. Since the coder processes do not share
ordinary memory, all inter-process data lives either in shared memory or in
named semaphores, and every child stops with `_exit`; the parent is the only
process responsible for the shared resources. Output is made unbuffered, so a
child that terminates never loses a log line.

### Changes made while merging

The mandatory implementation keeps its algorithm, its control flow, its
constants and its log format; the `coders/` tree is the original CODEXION code.
The merge only contains these minimal fixes:

* `in_use` was never initialised in `init_dongles`; it is now explicitly set to
  `false` (found by UndefinedBehaviorSanitizer: the original code read an
  uninitialised `bool`, which only worked by chance because fresh `malloc`
  pages are usually zeroed);
* the heap now breaks equal keys by coder id, as required by the subject for a
  deterministic EDF policy;
* two Norm fixes: `clean_up`/`error` moved from `utils.c` to `cleanup.c` to stay
  under five functions per file, and the mutex array of `init_mutexes` is
  declared and filled on separate lines.

## Blocking cases handled

* **Circular wait / deadlock prevention.** The odd/even dongle acquisition order
  removes the fourth Coffman condition: no cycle can exist in the wait-for
  graph. This is true for both backends, since the bonus backend keeps the same
  order.
* **Starvation prevention.** The per-dongle heap implements the required
  arbitration: arrival order for `fifo`, earliest burnout deadline for `edf`,
  with a deterministic `(key, coder id)` tie-break. No coder can be served
  repeatedly while another is waiting.
* **Cooldown handling.** A released dongle is unavailable until its
  `ready_time`; waiters use timed waits (condition variable or semaphore) so
  they wake up exactly when the cooldown expires instead of waiting for another
  broadcast. The bonus backend reserves the next owner immediately, which keeps
  a steady stream of grants.
* **Lost wake-up prevention (bonus).** Grant semaphores persist a posted token
  until the coder consumes it, and every wait is bounded (cooldown deadline or
  100 ms safety step), so a coder always re-checks the shared state and can
  never sleep through a grant or a stop condition.
* **Precise burnout detection.** Each backend has a monitor polling every
  500 µs; a burnout is reported once, within the 10 ms tolerance, before the
  simulation is stopped.
* **Log serialization.** One dedicated lock/semaphore around all output;
  nothing is printed after the burnout line.
* **Clean shutdown (bonus).** The first coder detecting its burnout kills its
  siblings; the supervisor reaps them, then unlinks the named semaphores and
  unmaps the shared memory. Fork failures kill the already-created children
  before reporting the error.
* **No semaphore locked by a dying process (bonus).** A coder joins its monitor
  thread before exiting, and a burnout monitor releases the log semaphore before
  `_exit`, so a process can never die while holding a named semaphore — which
  would otherwise freeze every other process.
* **Signal-safe stop (bonus).** The stop flag is read and written under the log
  semaphore, so all processes agree on when the simulation is over.

## Thread synchronization mechanisms

The mandatory backend (`codexion`) uses:

* **`pthread_mutex_t` per dongle** for the dongle state and its heap. Keeping
  contention local to one dongle lets coders whose dongles do not overlap
  compile fully in parallel.
* **`pthread_cond_t` per dongle** for the waiters. A coder pushes its request
  into the heap, broadcasts, and loops on `pthread_cond_timedwait` until it is
  at the top of the heap, the dongle is free, and `now >= ready_time`.
  `pthread_cond_timedwait` is what makes the cooldown autonomous: the waiter
  wakes up at the dongle's exact `ready_time`, even without another wake-up
  from a neighbour.
* **A single log mutex** for output, and **safe-access helpers** (`safe_read_*`
  / `safe_write_*`) around the deadline, the stop flags, the completion counter
  and the FIFO ticket, each with its own mutex. This guarantees that the value
  used as an EDF priority, the stop decision and the counters are always
  consistent between the coder threads and the monitor thread.
* **A monitor thread** that only reads deadlines through the deadline mutex and
  writes the stop flags through their mutexes, then exits; coder threads join
  the monitor so the whole simulation terminates cleanly.

Race conditions are prevented by making each shared value reachable only
through its mutex, and by never taking a dongle mutex while holding another
lock in the reverse order. Thread-safe communication between coders and the
monitor is done through the mutex-protected flags: the monitor sets
`one_burnout` (or `all_coders_compiled`) and every coder checks it before
logging or sleeping, so no signal is needed and no thread touches a dongle
after the stop decision.

The bonus backend (`codexion_bonus`) replaces those primitives as follows:

| Mandatory primitive                  | Bonus equivalent                                   |
|--------------------------------------|----------------------------------------------------|
| per-dongle `pthread_mutex_t`         | per-dongle named semaphore (value 1)               |
| per-dongle `pthread_cond_t`          | per-coder grant semaphore + `sem_timedwait`         |
| mutex-protected shared variables      | semaphores + shared memory (`mmap MAP_SHARED`)     |
| one monitor thread in the process     | one monitor thread **per coder process**           |
| `pthread_join` of all threads         | supervisor `waitpid` of all children               |

## Continuous integration

`.github/workflows/ci.yml` runs four jobs on every push and pull request:

1. **Build & functional tests** — `make`, a no-relink check
   (`make -q codexion codexion_bonus`), then the full `make test` suite.
2. **AddressSanitizer + UndefinedBehaviorSanitizer** — both binaries rebuilt
   with `-fsanitize=address,undefined`, then the same functional suite with
   leak detection enabled.
3. **ThreadSanitizer** — both binaries rebuilt with `-fsanitize=thread` and run
   through `tests/tsan.sh` (ASLR is relaxed for TSan on the runner).
4. **42 Norm** — `norminette` is installed and run on `coders` and
   `coders_bonus`; the whole code base is Norm-clean.

## Resources

* `man pthread_create`, `man pthread_mutex_lock`, `man pthread_cond_wait`,
  `man pthread_cond_timedwait`
* `man fork`, `man waitpid`, `man kill`, `man sem_open`, `man sem_wait`,
  `man sem_timedwait`, `man sem_post`, `man mmap`
* `man gettimeofday`, `man usleep`
* Coffman's conditions for deadlock (mutual exclusion, hold and wait, no
  preemption, circular wait) — used as a checklist for the dongle acquisition
  order
* POSIX threads and POSIX semaphores documentation
  ([pthreads(7)](https://man7.org/linux/man-pages/man7/pthreads.7.html),
  [sem_overview(7)](https://man7.org/linux/man-pages/man7/sem_overview.7.html),
  [shm_overview(7)](https://man7.org/linux/man-pages/man7/shm_overview.7.html))
* [The Little Book of Semaphores](https://greenteapress.com/wp/semaphores/)
* [Dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
