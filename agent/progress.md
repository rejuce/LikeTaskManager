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

## TODO / Next Steps
- [ ] Commit `CMakeLists.txt` + qmake removals on `Qt6Cmake` branch (**needs approval**)
- [ ] Update `Jenkinsfile` for CMake-based AppImage build (currently references `qmake` + `LikeTaskManager.pro`)
- [ ] Consider installing `qt6-l10n-tools` CMake package (`Qt6LinguistTools_DIR` missing) to enable translation generation
- [ ] Remove `google867aaac7f0fdc31e.html` (owner verification file, not needed in repo)
