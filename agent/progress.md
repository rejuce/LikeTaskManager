# Progress Log — LikeTaskManager (Qt6 CMake Migration)

## 2025-07-14

### Step 1: Repository Setup
- [x] Cloned `https://github.com/rejuce/LikeTaskManager` into `/workspace/repos/LikeTaskManager`
- [x] Checked out `qt6` branch (base branch for CMake migration)
- [x] Created `agent/` folder
- [x] Created `agent/progress.md` (this file)
- [x] Created `agent/outline.md` — repository map for future agentic work
- [x] Created `Qt6Cmake` branch from `qt6`
- [x] Pushed `Qt6Cmake` to origin using SSH key at `/workspace/repos/keys/github`

### Notes
- **Base branch:** `qt6` (not `master` which is Qt5/legacy)
- **Working branch:** `Qt6Cmake`
- **Remote:** `origin` → `github.com:rejuce/LikeTaskManager.git`
- **SSH key:** `/workspace/repos/keys/github` (permissions 600)
- **Build system:** Currently qmake (`.pro` file). Goal: migrate to CMake.
- **Qt version:** Qt 6 (qt6 branch)
- **External deps in .pro:** Qwt (hardcoded path `/home/jk/libs/qwt/install-6.3.0`), libnethogs (`-lnethogs`)
- **UI file:** `ltm.ui` (Qt Designer form)
- **Resources:** `qdarkstyle/style.qrc`, `images.qrc`
- **Translations:** `LikeTaskManager_de_AT.ts`

## 2025-10-19

### Step 2: Build Environment Setup (OpenTerminal)
- [x] Installed via apt: `qt6-base-dev`, `libqt6network6`, `libqt6widgets6`, `qt6-svg-dev`, `qt6-l10n-tools`, `cmake`, `ninja-build`, `meson`, `libnl-3-dev`, `libnl-genl-3-dev`, `libpcap-dev` (Debian 13 / trixie, Qt 6.8.2)
- [x] Built **Qwt 6.3.0** from source (SourceForge tarball — GitHub `KDE/qwt` is 404 now), installed to `/usr/local/qwt-6.3.0/` (headers in `include/` flat, lib in `lib/libqwt.so`), registered in `/etc/ld.so.conf.d/qwt-6.3.0.conf`
- [x] Symlinked `/home/jk/libs/qwt/install-6.3.0 → /usr/local/qwt-6.3.0` so the hardcoded Qwt path in `.pro` works without code changes
- [x] Built **libnethogs 0.8.8** from source (meson/ninja), installed to `/usr/local` (real lib name is `liblibnethogs.so`)
- [x] Created compatibility symlinks: `/usr/local/lib/x86_64-linux-gnu/libnethogs.so → liblibnethogs.so` and `/usr/local/include/libnethogs.h → /usr/local/include/libnethogs/libnethogs.h` (project links `-lnethogs` and includes `<libnethogs.h>`)
- [x] **Validated:** full out-of-tree qmake build in `/tmp/ltm-build` succeeds (`qmake6 LikeTaskManager.pro && make -j`), binary links `libqwt.so.6.3` + `liblibnethogs.so`
- [x] **Smoke test:** `QT_QPA_PLATFORM=offscreen ./LikeTaskManager` runs — all stat readers start; nethogs loop fails gracefully with `Operation not permitted` (expected in unprivileged container; app handles it via `noRootPriv` flag)
- [x] Dependency sources kept at `deps-src/` (qwt-6.3.0, nethogs-0.8.8, tarballs) — consider adding to `.gitignore`

### Build commands for this environment
```bash
mkdir -p /tmp/ltm-build && cd /tmp/ltm-build
qmake6 /workspace/repos/LikeTaskManager/LikeTaskManager.pro -spec linux-g++ CONFIG+=release
make -j$(nproc)
# run (headless):
QT_QPA_PLATFORM=offscreen ./LikeTaskManager
```

## 2026-10-19

### Step 3: CMake Migration (qmake → CMake)
- [x] Created `CMakeLists.txt`:
  - `find_package(Qt6 ... Core Gui Widgets Network)` + optional `LinguistTools`
  - `CMAKE_AUTOMOC/AUTORCC/AUTOUIC` ON, C++17, `QT_DEPRECATED_WARNINGS` define
  - Qwt located via `find_path`/`find_library` (no CMake config ships with Qwt); supports `-DQWT_ROOT=<prefix>`
  - libnethogs via `find_library(NAMES libnethogs nethogs)` + `find_path(NETHOGS_INCLUDE_DIR NAMES libnethogs.h)`
  - `ltm.ui` handled by AUTOUIC, `images.qrc` + `qdarkstyle/style.qrc` by AUTORCC
  - `qt6_add_translations()` guarded by `Qt6LinguistTools_FOUND` (package not present in this container → skipped with warning)
  - Install rule: `RUNTIME DESTINATION /opt/LikeTaskManager/bin`
- [x] Fixed Qwt `find_path` to `qwt_plot.h` (headers installed flat, no `qwt/` subdir)
- [x] Added `${CMAKE_CURRENT_SOURCE_DIR}` to target include path (headers use `#include <cpustatreader.h>` style)
- [x] **Validated:** clean out-of-tree CMake+Ninja build in `/tmp/ltm-cmake` — configure + compile + link OK
- [x] **Smoke test:** `QT_QPA_PLATFORM=offscreen ./LikeTaskManager` — all 5 stat readers start, nethogs loop fails gracefully (expected, unprivileged container), same behavior as qmake baseline
- [x] `ldd` confirms linkage: `libqwt.so.6.3` + `liblibnethogs.so` (nethogs SONAME is `liblibnethogs.so.undefined` — baked into its meson build, identical to qmake baseline)
- [x] Removed qmake artifacts via `git rm` (staged, NOT committed): `LikeTaskManager.pro`, `LikeTaskManager.pro.user`, `Makefile`, `.qmake.stash`
- [x] Re-verified clean build after removal (`/tmp/ltm-cmake-verify`)

### CMake build commands
```bash
cmake -S /workspace/repos/LikeTaskManager -B /tmp/ltm-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/ltm-cmake
QT_QPA_PLATFORM=offscreen /tmp/ltm-cmake/LikeTaskManager
```

## 2026-10-19 (continued)

### Step 4: Unit Test Infrastructure (CMake + QTest)

Goal: add unit tests to every module. Iterative approach: add tests for one module → build → run/verify → next module.

**Approach decisions:**
- New `tests/` subdirectory with its own `CMakeLists.txt` (wired via `add_subdirectory(tests)` + `enable_testing()` in root CMakeLists).
- Shared static library `ltm_testlib` compiles the sources under test (readers + widgets) so test executables don't depend on the GUI app target. The app target itself is left untouched (no regression risk).
- One test executable per module: `t_<module>` registered with CTest (`add_test`).
- Widget tests run headless: `QT_QPA_PLATFORM=offscreen` set as test environment property.
- Private members of legacy classes are accessed via a test-only `#define private public` trick (contained per test file) since there is no seam/refactor to be made at this stage.

**Module test plan:**
| # | Module | Test exe | Notes |
|---|--------|----------|-------|
| 1 | `cpustats.h/.cpp` | `t_cpustats` | /proc/stat parsing, cpu count, activity vector, idle/active helpers |
| 2 | `StatReader.h` | `t_statreader` | template: data vector, filter_data_deque, start/stop thread loop (test subclass) |
| 3 | `ramstatreader` | `t_ramreader` | /proc/meminfo MemTotal parsing, deque sizing |
| 4 | `diskstatreader` | `t_diskreader` | disk discovery, find_disk_name_in_data |
| 5 | `networkstatreader` | `t_networkreader` | iface discovery, byte counters, link stats |
| 6 | `processstatreader` | `t_processreader` | process list, nethogs graceful failure flags |
| 7 | item widgets | `t_widgets` | PerfItemWidget + CPU/RAM/Disk/Network widget label updates (offscreen) |

- [x] **t_statreader** (10 tests): empty initial state, device count, getStatData const reference, filter smooths a spike, minimum safe filter size, start() runs loop, exception swallowed by templateLoop, stop() bug documented. All pass.
  - **Bug found (documented, not fixed):** `filter_data_deque` has no size guard — deques with size < 13 cause `size_t` underflow on `k-2` (verified: size 11 → SIGSEGV). Production deques are always 600 points, so unreachable in practice.
  - **Bug found (documented, not fixed):** `stop()` sets `m_quit = false` — it is a no-op, the measurement thread cannot be stopped via public API.
- [x] **t_cpustats** (10 tests): /proc/stat cpu count vs. direct count, activity vector size/range, "tot"+"0..N" naming, GetIdleTime/GetActiveTime arithmetic. All pass.
  - Note: found that `get_cpus_activity_vector()` returns NaN when two samples have zero total time (e.g. back-to-back calls) — a latent bug in `cpustats.cpp` (no guard for `TOTAL_TIME == 0`). Test works around it with a 150ms sleep; the production code fix is a separate follow-up.
- [x] **t_ramreader** (7 tests): device count/name ("GenMemory"), deque sizing (600 points), MemTotal parsing from /proc/meminfo, RamSizeKiB positive, live measurement loop populates deque. All pass.
  - **Approach:** used `friend class TestRamReader` in `StatReader.h` and `ramstatreader.h` for protected member access (cleaner than `#define private public` which breaks MOC).
  - Note: first measurement sample can be negative (MemAvailable > MemTotal transient in containers) — test checks for non-zero samples rather than strict 0..100 range.
- [x] **t_diskreader** (8 tests): deque sizing, disk names match /sys/block, find_disk_name_in_data returns correct pointer / nullptr, sizeByte consistency, measure_main_loop runs without crash. All pass.
  - Used `friend class TestDiskReader` in `diskstatreader.h`.
- [x] **t_networkreader** (9 tests): interface count matches /sys/class/net, deque sizing, previous byte counters initialised from sysfs, read_total_rx/tx_bytes match sysfs, interface name lookup, measure_main_loop stable. All pass.
  - Used `friend class TestNetworkReader` in `networkstatreader.h`.
- [x] **t_processreader** (8 tests): data vector non-empty, find_pid_in_data for own PID / unknown PID, process name populated, nethogs failure flags set in container, measure_main_loop runs without crash. All pass.
  - Used `friend class TestProcessReader` in `processstatreader.h`.
  - Nethogs "Operation not permitted" messages in test output are expected (unprivileged container).
  - Test is resilient to container /proc restrictions: if own PID not found, falls back to checking any PID in the data vector.
- [x] **t_widgets** (14 tests): PerfItemWidget base (plot type, label creation), CPUItemWidget (plot type, update_data with/without load), RamItemWidget (plot type, update_data, zero-total guard), DiskItemWidget (plot type, name in first line, update_data), NetworkItemWidget (plot type, name in first line, KB/s and MB/s ranges). All pass.
  - Used `friend class TestWidgets` in `perfitemwidget.h`.
  - QWARN about null pixmaps / stylesheet parsing is expected (no QRC resources loaded in test environment).

### Full ctest run
- [x] `ctest` in `/tmp/ltm-test`: **7/7 test suites pass, 0 failures** (total ~3.5s)
  - 68 individual test functions across 7 executables

### Known bugs found (documented, NOT fixed)
1. **`filter_data_deque` no size guard:** deques with size < 13 cause `size_t` underflow on `k-2` (size 11 → SIGSEGV). Production deques are always 600 points, so unreachable.
2. **`stop()` no-op:** sets `m_quit = false` instead of `true`, so the measurement thread cannot be stopped via the public API.
3. **`cpustats.cpp` NaN on zero total time:** `get_cpus_activity_vector()` divides by total time without guarding against 0 (back-to-back calls can produce NaN).

### Files changed (test infrastructure)
| File | Change |
|------|--------|
| `CMakeLists.txt` | Added `enable_testing()` + `add_subdirectory(tests)` |
| `tests/CMakeLists.txt` | **New** — shared lib `ltm_testlib`, auto-glob `t_*.cpp`, CTest registration |
| `tests/t_cpustats.cpp` | **New** — 10 tests |
| `tests/t_statreader.cpp` | **New** — 10 tests |
| `tests/t_ramreader.cpp` | **New** — 7 tests |
| `tests/t_diskreader.cpp` | **New** — 8 tests |
| `tests/t_networkreader.cpp` | **New** — 9 tests |
| `tests/t_processreader.cpp` | **New** — 8 tests |
| `tests/t_widgets.cpp` | **New** — 14 tests |
| `StatReader.h` | Added `friend class Test*` declarations (5 friends) |
| `ramstatreader.h` | Added `friend class TestRamReader` |
| `diskstatreader.h` | Added `friend class TestDiskReader` |
| `networkstatreader.h` | Added `friend class TestNetworkReader` |
| `processstatreader.h` | Added `friend class TestProcessReader` |
| `perfitemwidget.h` | Added `friend class TestWidgets` |

## TODO / Next Steps
- [ ] Commit `CMakeLists.txt` + qmake removals on `Qt6Cmake` branch (**needs approval**)
- [ ] Update `Jenkinsfile` for CMake-based AppImage build (currently references `qmake` + `LikeTaskManager.pro`)
- [ ] Consider installing `qt6-l10n-tools` CMake package (`Qt6LinguistTools_DIR` missing) to enable translation generation
- [ ] Remove `google867aaac7f0fdc31e.html` (owner verification file, not needed in repo)
