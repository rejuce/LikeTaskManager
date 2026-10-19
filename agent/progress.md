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

- [x] **t_statreader** (10 tests): empty initial state, device count, getStatData const reference, filter smooths a spike, minimum safe filter size (verifies no-op for < 13), start() runs loop, stop() stops thread (verifies join), exception swallowed by templateLoop. All pass.
  - **Bug found & fixed:** `filter_data_deque` size guard added — deques < 13 now return unchanged (was `size_t` underflow → SIGSEGV).
  - **Bug found & fixed:** `stop()` now sets `m_quit=true` and joins the thread (was a no-op setting `m_quit=false`).
- [x] **t_cpustats** (10 tests): /proc/stat cpu count vs. direct count, activity vector size/range, "tot"+"0..N" naming, GetIdleTime/GetActiveTime arithmetic, back-to-back call returns finite values (no NaN). All pass.
  - **Bug found & fixed:** `get_cpus_activity_vector()` now returns 0.0f when `TOTAL_TIME == 0` (was NaN from divide-by-zero on back-to-back calls).
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

### Bugs found and fixed
1. **`filter_data_deque` no size guard (FIXED):** deques with size < 13 caused `size_t` underflow on `k-2` (size 11 → SIGSEGV). Fixed by adding `if(data.size() < 13) return;` guard at the top of the function. Test `filter_requires_minimum_size` now verifies no-op for sizes 5 and 12.
2. **`stop()` no-op (FIXED):** was setting `m_quit = false` (no-op), so the measurement thread could not be stopped. Fixed to set `m_quit = true` and join the thread. Tests `start_runs_loop` and `stop_stops_thread` now verify correct stop behavior.
3. **`cpustats.cpp` NaN on zero total time (FIXED):** `get_cpus_activity_vector()` divided by total time without guarding against 0 (back-to-back calls produced NaN). Fixed by adding `if(TOTAL_TIME > 0.f)` guard, returning 0.0f when no time elapsed. Test `idle_plus_active_covers_time` now calls back-to-back (no sleep) and asserts all values are finite.

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

## 2026-10-19 (continued)

### Step 5: GitHub Actions AppImage Workflow

Goal: replace the old qmake-based Jenkins pipeline with a GitHub Actions workflow that builds the AppImage. **Local AppImage build was intentionally NOT pursued** (container has no FUSE device / Docker; host glibc 2.41 is too new for linuxdeployqt) — GitHub does the lifting on `ubuntu-22.04` (glibc 2.35, the compat sweet spot).

- [x] Created `LikeTaskManager.desktop` (was missing from repo; required by linuxdeployqt for AppDir bundling)
- [x] Created `.github/workflows/build-appimage.yml`:
  - `runs-on: ubuntu-22.04` **without** a container override — the runner image itself ships glibc 2.35, which linuxdeployqt requires (≥ 2.36 trips the "new glibc" guard)
  - Deps: Qt6 dev, Qwt built from SourceForge tarball → `/usr/local` (Qwt ships no CMake config)
  - **libnethogs 0.8.8** from GitHub tarball (meson), plus:
    - `patchelf --set-soname liblibnethogs.so liblibnethogs.so` — its meson build bakes in SONAME `liblibnethogs.so.undefined`, which makes linuxdeployqt bundle a useless `liblibnethogs.so.undefined` file
    - compat symlinks `libnethogs.so` + flat `libnethogs.h` (project links `-lnethogs`)
  - CMake build + `ctest`
  - AppDir prep (binary, `.desktop`, icon) → `linuxdeployqt-continuous` AppImage with `-appimage -bundle-non-qt-libs`
  - **Smoke test:** runs the AppImage for 10 s under `xvfb-run` and fails if it exits early (FUSE mounting works on GitHub runners)
  - Uploads `LikeTaskManager-*.AppImage` as artifact

### Local validation performed (pre-GitHub)
- [x] Full CMake build of the app in this container: configure + compile + link OK
- [x] Replicated the bundling steps: built linuxdeployqt **from source** (AppImage runtime can't mount here — no FUSE), ran it against a prepared AppDir → all Qt6 libs/plugins bundled correctly, `qt.conf` + icon + desktop entries in place
- [x] **Found & fixed the nethogs SONAME bug** (would have shipped a broken AppImage): verified locally with `patchelf` that `NEEDED liblibnethogs.so` resolves in the AppDir
- [x] AppDir sanity check: `ldd` on bundled binary resolves everything inside the AppDir (only glibc/libstdc++/libm/libgcc from the base system, as intended)
- [x] Workflow YAML validated with `pyyaml`
- [ ] **Not verified locally:** actual AppImage file creation + runtime smoke test (needs FUSE + glibc ≤ 2.35) → this is what the GitHub run will prove

### Files changed
| File | Change |
|------|--------|
| `.github/workflows/build-appimage.yml` | **New** — full build + bundle + smoke test + artifact pipeline |
| `LikeTaskManager.desktop` | **New** — required by linuxdeployqt (AppDir `/usr/share/applications/`) |

## TODO / Next Steps
- [ ] Commit `CMakeLists.txt` + qmake removals + `.github/workflows/build-appimage.yml` + `LikeTaskManager.desktop` on `Qt6Cmake` branch (**needs approval**), push, and watch the first CI run
- [ ] On first CI failure: likely candidates are Qwt tarball URL / nethogs tag availability — both pinned in `env:`
- [ ] Update `Jenkinsfile` for CMake-based AppImage build (currently references `qmake` + `LikeTaskManager.pro`) — or retire it in favour of GitHub Actions
- [ ] Consider installing `qt6-l10n-tools` CMake package (`Qt6LinguistTools_DIR` missing) to enable translation generation
- [ ] Remove `google867aaac7f0fdc31e.html` (owner verification file, not needed in repo)
