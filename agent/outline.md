# LikeTaskManager — Repository Map

> Quick reference for agents working on this codebase.
> Branch: `qt6` (base) → `Qt6Cmake` (working)

## Project Overview
**LikeTaskManager** is a Qt-based system performance monitor for Linux, mimicking Windows 10 Task Manager. It displays real-time CPU, RAM, Disk I/O, and Network utilisation (system-wide and per-process). Self-contained as an AppImage.

## Directory Structure

```
LikeTaskManager/
├── agent/                    # ← Agent working files (this folder)
│   ├── progress.md           # Progress log & step tracking
│   └── outline.md            # This file — repo map
│
├── main.cpp                  # Entry point — QApplication setup, loads qdarkstyle, shows LTM window
├── ltm.h / ltm.cpp           # Main window class (QMainWindow subclass) — orchestrates all widgets & readers
├── ltm.ui                    # Qt Designer UI form (only form in project)
│
│── ── Stat Readers (data collection, run in background threads) ──
├── StatReader.h              # Template base class <T> — thread loop, data deque, mutex, filter
│
├── cpustatreader.h/.cpp      # CPUStatReader : StatReader<CPUData> — reads /proc/stat per-core
├── cpustats.h/.cpp           # CPUData struct + CPU stats helpers
├── ramstatreader.h/.cpp      # RamStatReader : StatReader<RamData> — reads /proc/meminfo
├── diskstatreader.h/.cpp     # DiskStatReader : StatReader<DiskData> — reads /proc/diskstats
├── networkstatreader.h/.cpp  # NetworkStatReader : StatReader<NetworkData> — reads /sys/class/net/
├── processstatreader.h/.cpp  # ProcessStatReader : StatReader<ProcessData> — /proc + libnethogs
├── processgroup.h/.cpp       # ProcessGroup struct + aggregate_processes() — groups same-name PIDs
│
│── ── Item Widgets (UI panels, one per stat category) ──
├── cpuitemwidget.h/.cpp      # CPU panel: usage %, clock speed, core count, QwtPlot
├── ramitemwidget.h/.cpp      # RAM panel: usage, swap, QwtPlot
├── diskitemwidget.h/.cpp     # Disk panel: per-disk read/write, IOPS, QwtPlot
├── networkitemwidget.h/.cpp  # Network panel: per-iface tx/rx, link speed, QwtPlot
├── perfitemwidget.h/.cpp     # Performance item widget (process tab item)
│
│── ── Tab-specific pages (QStackedWidget pages) ──
├── processsorttree.h/.cpp    # ProcessSortTree (QTreeWidget subclass) — manual numeric column sort
├── ltm_cpu.cpp               # CPU tab page logic
├── ltm_ram.cpp               # RAM tab page logic
├── ltm_disk.cpp              # Disk tab page logic
├── ltm_network.cpp           # Network tab page logic
│
│── ── Resources & Assets ──
├── images.qrc                # Qt resource file: cpu.png, ram.png, hard-disk.png, ethernet.png, gpu.png
├── qdarkstyle/               # QDarkStyleSheet (dark theme)
│   ├── style.qss             # Main stylesheet
│   ├── style.qrc             # Resource file for theme icons
│   └── rc/                  # ~100 PNG icons for the dark theme
│
│── ── Build / Config ──
├── CMakeLists.txt            # CMake project file (qmake .pro removed — staged deletion on Qt6Cmake)
├── tests/
│   ├── CMakeLists.txt        # Test infra: shared lib ltm_testlib, auto-glob t_*.cpp, CTest
│   ├── t_cpustats.cpp        # 10 tests — /proc/stat parsing, CPU helpers
│   ├── t_statreader.cpp      # 10 tests — StatReader<T> template (dummy subclass)
│   ├── t_ramreader.cpp       # 7 tests — /proc/meminfo, deque sizing
│   ├── t_diskreader.cpp      # 8 tests — disk discovery, find_disk_name_in_data
│   ├── t_networkreader.cpp   # 9 tests — iface discovery, byte counters
│   ├── t_processreader.cpp   # 8 tests — process list, nethogs graceful failure
│   ├── t_processsorttree.cpp # 6 tests — numeric sort + expansion preservation
│   └── t_widgets.cpp         # 14 tests — PerfItemWidget + CPU/RAM/Disk/Network widgets
├── Jenkinsfile               # CI pipeline config (legacy qmake — superseded by GitHub Actions)
├── .github/workflows/
│   └── build-appimage.yml    # GitHub Actions: Qt6 + Qwt + libnethogs build → AppImage + smoke test + artifact
├── LikeTaskManager.desktop   # Desktop entry (required by linuxdeployqt for AppImage bundling)
│
│── ── Misc ──
├── LikeTaskManager_de_AT.ts  # German (Austria) translation
├── README.md                 # Project description
├── ltm.PNG, ltm2.PNG, ltm3.PNG  # Screenshot images for README
├── google867aaac7f0fdc31e.html   # Google ownership verification (not needed)
└── LikeTaskManager/          # Subfolder (appimage build output?)
```

## Key Architecture

### Data Flow
```
/proc, /sys (Linux kernel)
        │
        ▼
StatReader<T> (template, background thread, 100ms interval)
  ├── CPUStatReader
  ├── RamStatReader
  ├── DiskStatReader
  ├── NetworkStatReader
  └── ProcessStatReader (uses libnethogs for per-process network)
        │
        │  emit data_ready() signal
        ▼
LTM (main window) — slots: plot_cpu_activity(), plot_ram_activity(), etc.
        │
        ▼
ItemWidgets (QwtPlot curves) + static labels
```

### Key Classes & Where to Find Them

| Class | File | Purpose |
|-------|------|---------|
| `LTM` | `ltm.h` / `ltm.cpp` | Main window. Wires up all readers, widgets, QwtPlot curves. Has slots for each stat category. |
| `StatReader<T>` | `StatReader.h` | Template base: thread loop (100ms), deque of data points (600/min), mutex, 5-point moving average filter. |
| `CPUStatReader` | `cpustatreader.h/.cpp` | Reads `/proc/stat` for per-core CPU usage and clock speed. |
| `RamStatReader` | `ramstatreader.h/.cpp` | Reads `/proc/meminfo` for RAM and swap usage. |
| `DiskStatReader` | `diskstatreader.h/.cpp` | Reads `/proc/diskstats` for per-disk I/O. Dynamically discovers disks. |
| `NetworkStatReader` | `networkstatreader.h/.cpp` | Reads `/sys/class/net/` for per-interface tx/rx. Dynamically discovers interfaces. |
| `ProcessStatReader` | `processstatreader.h/.cpp` | Reads `/proc/[pid]/` + libnethogs for per-process CPU, memory, disk, network. |
| `ProcessGroup` | `processgroup.h/.cpp` | Groups same-named processes: aggregated CPU/mem/disk/net + count. Used by QTreeWidget. |
| `CPUItemWidget` | `cpuitemwidget.h/.cpp` | CPU tab UI: core count, usage %, clock speed, QwtPlot. |
| `RamItemWidget` | `ramitemwidget.h/.cpp` | RAM tab UI: total/used/swap, QwtPlot. |
| `DiskItemWidget` | `diskitemwidget.h/.cpp` | Disk tab UI: per-disk read/write rate, IOPS, QwtPlot. |
| `NetworkItemWidget` | `networkitemwidget.h/.cpp` | Network tab UI: per-iface tx/rx, link speed, QwtPlot. |
| `PerfItemWidget` | `perfitemwidget.h/.cpp` | Process list item widget. |
| **Process tab (QTreeWidget)** | `ltm.cpp` `handle_process_data()` | Groups by name, expandable per-PID children. Kill button kills group or single PID. Items updated in-place (no clear+rebuild). |
| `ProcessSortTree` | `processsorttree.h/.cpp` | QTreeWidget subclass with manual numeric column sort. Built-in sort always disabled; `std::stable_sort` on `UserRole` doubles. Header clicks via `sectionPressed`. `applyNumericSort()` called each update cycle. `sortByColumn()` recursively saves/restores expansion state (Qt's `takeTopLevelItem` collapses items + descendants). |

### Qt Modules Used
- `core`, `gui`, `widgets`, `network`, `test` (Qt6::Core/Gui/Widgets/Network/Test)
- External: **Qwt 6.3.0** (plotting, located via `find_path`/`find_library`, `-DQWT_ROOT` supported), **libnethogs** (per-process network stats)

### Unit Test Infrastructure (DONE 2026-10-19, extended 2026-10-23)
- **9 test executables**, 73 test functions, all passing via `ctest`
- Shared static lib `ltm_testlib` compiles reader + widget sources (no GUI app dependency)
- Private member access via `friend class Test<X>` declarations in headers
- Widget tests run offscreen: `QT_QPA_PLATFORM=offscreen`
- Build & run: `cmake -S <repo> -B /tmp/ltm-test -G Ninja && cmake --build /tmp/ltm-test && cd /tmp/ltm-test && ctest --output-on-failure`

#### Bugs found and fixed
1. `StatReader::filter_data_deque` — size guard added: deques < 13 elements return unchanged (was `size_t` underflow / SIGSEGV).
2. `StatReader::stop()` — now sets `m_quit=true` and joins the thread (was a no-op setting `m_quit=false`).
3. `cpustats.cpp` `get_cpus_activity_vector()` — returns 0.0f when total time is 0 (was NaN from divide-by-zero).

### UI
- Single Qt Designer form: `ltm.ui` → compiled into `Ui::LTM`
- Dark theme: `qdarkstyle/style.qss` loaded at startup in `main.cpp`

## CMake Migration Notes (DONE 2026-10-19)

`CMakeLists.txt` replicates everything from the old `.pro` file:
- Qt6 components Core/Gui/Widgets/Network + optional LinguistTools (skipped with warning when absent)
- C++17, AUTOMOC/AUTORCC/AUTOUIC, `QT_DEPRECATED_WARNINGS`
- `ltm.ui` (AUTOUIC), `images.qrc` + `qdarkstyle/style.qrc` (AUTORCC)
- Qwt: `find_path(qwt_plot.h)` + `find_library(qwt)` — Qwt ships no CMake config; search hints cover `/usr/local/qwt-6.3.0`; override with `-DQWT_ROOT`
- libnethogs: `find_library(NAMES libnethogs nethogs)` + `find_path(libnethogs.h)`
- Translation: `qt6_add_translations()` guarded by `Qt6LinguistTools_FOUND`
- Install: `RUNTIME DESTINATION /opt/LikeTaskManager/bin`
- Qmake artifacts (`LikeTaskManager.pro`, `.pro.user`, `Makefile`, `.qmake.stash`) removed via `git rm` (staged, uncommitted)

### Build commands
```bash
cmake -S <repo> -B /tmp/ltm-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/ltm-cmake
QT_QPA_PLATFORM=offscreen /tmp/ltm-cmake/LikeTaskManager   # headless smoke test
```

### Known quirks
- `Qt6LinguistTools` CMake package not installed in container (binaries exist at `/usr/lib/qt6/bin/`) → translation generation skipped
- nethogs lib ships with SONAME `liblibnethogs.so.undefined` (its meson build) — **must be fixed with `patchelf --set-soname liblibnethogs.so`** before AppImage bundling, otherwise linuxdeployqt bundles a file under the `.undefined` name. The GitHub Actions workflow does this at build time.
- `Jenkinsfile` is still qmake-based and has been superseded by `.github/workflows/build-appimage.yml`

## GitHub Actions AppImage Build (ADDED 2026-10-19, CI ITERATION 2026-10-19)

`.github/workflows/build-appimage.yml` — runs on `ubuntu-22.04` **without a container** (runner image glibc 2.35 is the max linuxdeployqt supports): install deps → build Qwt 6.3.0 (SourceForge tarball, `qmake6`, OpenGL disabled via `sed` on `qwtconfig.pri`) → build libnethogs **v0.8.8** from **`raboof/nethogs`** (meson, `.so` copied from build tree, SONAME fixed via patchelf, compat symlinks) → CMake build → ctest → AppDir + linuxdeployqt-continuous → smoke test under `xvfb-run` → upload AppImage artifact. Triggers on push/PR to `qt6`/`Qt6Cmake` + manual dispatch. Local AppImage build is not possible in the dev container (no FUSE, glibc 2.41).

### CI Fix History (8 pushes to `Qt6Cmake`)
| Commit | Fix |
|--------|-----|
| `acc2fd1` | `qt6-svg-dev` → `libqt6svg6-dev` (Ubuntu 22.04 package name) |
| `6d5e1f0` | Qwt: `qmake6` build (no CMake in Qwt 6.x), added `make` to deps |
| `303d1ec` | Qwt OpenGL: `sed qwtconfig.pri` (command-line vars don't work) |
| `8c4c5d0` | Added `libgl-dev` (Qt6::Gui transitive libGL) |
| `72ae939` | libnethogs: dynamic `.so` path lookup |
| `0f83bbc` | libnethogs: explicit search + diagnostics |
| `696b2aa` | **Root fix:** repo `ntop`→`raboof`, tag `0.8.8`→`v0.8.8`, header `src/libnethogs.h`, skip `meson install` |
| `ec769dd` | Removed invalid `-Ddisable-wifi/-Ddisable-nvidia` meson options (not in v0.8.8) |
| `dc606c5` | Symlink `qmake6`→`qmake` in `/usr/local/bin` (Qt5 is default on ubuntu-22.04) |
| `25e1878` | `chmod +x` AppImage after bundling (linuxdeployqt omits exec bit) |
| `88afb77` | Smoke test: `--appimage-extract` + run `AppRun` (no FUSE on GH runners) |

### Build Environment (OpenTerminal container, Debian 13)
- **Qt 6.8.2** via apt (`qt6-base-dev`, `libqt6network6`, `libqt6widgets6`, `qt6-svg-dev`, `qt6-l10n-tools`)
- **Qwt 6.3.0** built from SourceForge tarball → installed at `/usr/local/qwt-6.3.0/`; symlink `/home/jk/libs/qwt/install-6.3.0` → it (matches hardcoded path in `.pro`)
- **libnethogs 0.8.8** built from source (meson) → `/usr/local`; real lib is `liblibnethogs.so`, with compat symlinks `libnethogs.so` and `/usr/local/include/libnethogs.h`
- Toolchain: g++ 14.2, cmake, ninja, meson, make
- Build (CMake): `cmake -S <repo> -B /tmp/ltm-cmake -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/ltm-cmake`
- Run headless: `QT_QPA_PLATFORM=offscreen ./LikeTaskManager` (nethogs loop needs CAP_NET_ADMIN/root — degrades gracefully without it)
- Dependency sources kept at `deps-src/` (untracked)

## Git Info
- **Remote:** `origin` → `github.com:rejuce/LikeTaskManager.git`
- **Branches:** `master` (Qt5 legacy), `Qt5-final`, `qt6` (base), `Qt6Cmake` (working), `qtree-process-view` (Issue #12)

## Issue #12 — Process Grouping (Branch: `qtree-process-view` → merged to `master`)
- **Status:** Implemented, visually verified on Xvfb. All 8 tests pass. Pending: commit & push.
- **New files:** `processgroup.h`, `processgroup.cpp`, `processsorttree.h`, `processsorttree.cpp`, `tests/t_processgroup.cpp`
- **Modified:** `ltm.h`, `ltm.cpp`, `ltm.ui` (QTableWidget → QTreeWidget), `CMakeLists.txt`, `tests/CMakeLists.txt`
- **Screenshots:** `screenshot_processes_collapsed.png`, `screenshot_processes_grouped.png` (repo root, untracked)
- **SSH key:** `/workspace/repos/keys/github`
