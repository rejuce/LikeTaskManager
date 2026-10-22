# LikeTaskManager — Progress

## 2026-10-22: QTreeWidget fixes — numeric column sort + preserve expansion state (Bug A & B)

**Objective:** Fix two bugs in the Processes tab `QTreeWidget`:
- **Bug A:** Column header sorting used lexicographic (string) order instead of numeric order.
- **Bug B:** Tree expansion state was lost on every 100 ms data update (clear+rebuild).

**Solution:**
- **New files:** `processsorttree.h`, `processsorttree.cpp` — `ProcessSortTree` (QTreeWidget subclass)
  - Built-in sorting is **always disabled** (`setSortingEnabled(false)`) to prevent lexicographic re-sorts.
  - Header clicks handled via `QHeaderView::sectionPressed` (works with sorting disabled).
  - Manual numeric sort using `std::stable_sort` with a comparator that reads `Qt::UserRole` data via `toDouble(&ok)`, falling back to string comparison.
  - `applyNumericSort()` called at end of every `handle_process_data()` cycle.
  - Reordering uses `takeTopLevelItem(0)` / `addTopLevelItem(item)` with sorting disabled — preserves item identity and child expansion.
- **Modified files:**
  - `ltm.cpp` — Removed `setSortingEnabled(true)`; `handle_process_data()` updates items **in-place** (no delete/recreate); calls `ui->treeWProcess->applyNumericSort()` before re-enabling updates.
  - `CMakeLists.txt` — Added `processsorttree.cpp` / `processsorttree.h`.

**Verification:**
- All 8 unit tests pass (`ctest`)
- Release build succeeds (CMake + Ninja)
- Xvfb verification: Bug B fixed (tree stays expanded across 30+ update ticks)
- Xvfb verification: Bug A fixed (debug output confirms correct numeric before/after ordering; sort indicator moves on header click)
- **Status:** Complete. Pending: commit & push to `master`.

## 2026-10-22: Tagged v4.0.2 + added tag trigger & release job to master workflow
- Latest master build tagged **v4.0.2** (annotated). `v4.0.1` kept as-is on old
  Qt6Cmake commit `a0f0b90` (per decision; no v4.0.1 GitHub Release existed).
- **Problem:** the master workflow had no tag trigger and no `release:` job, so the
  initial tag push (at `06dddba`) did not run CI.
- **Fix:** ported from Qt6Cmake workflow (commit `d7c9cab`): `tags: [ 'v*' ]` trigger,
  `permissions: contents: write`, and a `release:` job (publishes AppImage + .zsync
  as GitHub Release via softprops/action-gh-release, on v* tags / manual dispatch).
- **Tag re-pointed:** `v4.0.2` force-pushed from `06dddba` to `d7c9cab` so the tag
  run uses the fixed workflow. This triggers build + release jobs.


## 2026-10-22: master — drop system `zsync` step (linuxdeployqt already makes .zsync)
- **Root cause of CI failure:** ubuntu-22.04's system `zsync` is old (0.0.8): it
  rejects `--make-sync-file` (`invalid option -- '-'`) and its `MakeSyncFile`
  subcommand also fails. Version-dependent, unreliable.
- **Fix:** Removed the standalone zsync step entirely. linuxdeployqt already
  generates `LikeTaskManager-x86_64.AppImage.zsync` during bundling (embedded
  `zsyncmake` + `-updateinformation` → log: "hence generating zsync file").
  The step now only verifies the file exists. `zsync` dropped from apt deps.
- Change only in `.github/workflows/build-appimage.yml`. YAML validated.
- **Committed & pushed to `origin/master`** (via SSH key `/workspace/repos/keys/github`).

## 2026-10-22: master — bring AppImage workflow to parity + fix `zsync MakeSyncFile` (exit 3)
- Reset local `master` to `origin/master` (`490fb83`) and committed directly there.
- **Root cause of CI failure:** modern `zsync` (0.8.x on ubuntu-22.04) removed the
  `MakeSyncFile` subcommand → `Usage: zsync http://...` exit 3. Use `zsync --make-sync-file <file>`.
- **Also ported from Qt6Cmake into the master workflow:**
  - `zsync` added to the apt dependency list
  - fixed AppImage name `LikeTaskManager-x86_64.AppImage` (guarded `mv` against
    "are the same file")
  - `-updateinformation` zsync URL (autoupdate from latest GitHub Release)
  - `Create .zsync file` step with `zsync --make-sync-file`
  - `.zsync` artifact upload
- Change only in `.github/workflows/build-appimage.yml`. YAML validated.
- **Committed & pushed to `origin/master`** (via SSH key `/workspace/repos/keys/github`).

## Current Focus: Debugging Segfault on Lubuntu 24.04 (headless/VNC)

### Status: Segfault root cause identified and fixed

### Segfault Fix (2025-06-15)
- **Root cause:** `ProcessStatReader::update_process_list()` had three bugs:
  1. `getpwuid()` can return `nullptr` (kernel threads, unknown UIDs) → null deref on `user->pw_name`
  2. Broken removal logic: outer `/proc` loop index `i` used as index into `m_DataVec` (different collection)
  3. `stringlist.at(1)` can throw on malformed `/proc` lines
- **Fix:** Rewrote `update_process_list()`:
  - Build a `QSet<int> livePids` from `/proc`, remove stale entries in reverse, add new PIDs
  - Guard `getpwuid()` result — fall back to numeric UID string if null
  - Guard `stringlist.at(1)` with `size() >= 2` checks
- **Verification:** Full local build passes, all 7 unit tests pass.

### Build Fix (2025-06-15)
- **Root cause:** `qdarkstyle/QDarkStyle.h` and `QDarkStyle.cpp` were never committed to git.
  The `qdarkstyle/` dir only had `rc/` images, `style.qrc`, and `style.qss`.
  CI failed with `QDarkStyle.h: No such file or directory`.
- **Fix:** Inlined `loadQDarkStyleSheet()` helper in `main.cpp` that reads
  `qrc:/qdarkstyle/style.qss` via `QFile`. No separate class files needed.
- **Also fixed in main.cpp:**
  - `backtrace_symbols_fd(frames, n, stderr)` → `fileno(stderr)` (needs `int fd`, not `FILE*`)
  - Added `#include <QScreen>` (was only forward-declared via `QApplication`)
- **Removed:** `qdarkstyle/QDarkStyle.h`, `qdarkstyle/QDarkStyle.cpp` (unnecessary)
- **Verification:** Full local build passes, all 7 unit tests pass.

### Problem
AppImage segfaults on Lubuntu 24.04 headless server (xserver dummy screen, VNC only).
Works fine on Ubuntu 22.04 desktop VM.

Crash occurs after "instantiating MainWindow" but before "loading qDarkStylesheet..." —
i.e., inside `LTM::LTM()` constructor.

### Debug Hypotheses (ranked by likelihood)
1. **`ui->listWidget->item(0)` is nullptr** → `dynamic_cast<PerfItemWidget*>(nullptr)` → 
   `perfItemWidetPtr->getPlotType()` → **SEGFAULT**. 
   Could happen if the system has no block devices or net interfaces visible, leaving the list empty.
2. **QwtPlot `replot()` crash** during setup with dummy X screen (less likely since it works on Xvfb in CI).
3. **QDarkStyleSheet** issue — unlikely since crash is before that print.

### Actions Taken
- [x] Added granular `std::cout` debug prints in `ltm.cpp` LTM constructor (every step)
- [x] Added null-check guard in `on_listWidget_itemClicked()` — if item is null or dynamic_cast fails, print & return
- [x] Added empty-listWidget guard in constructor — if count()==0, print FATAL and skip selection
- [x] Added `std::cout` prints in `main.cpp` around QApplication, LTM ctor, stylesheet, show(), exec()
- [x] Added **SIGSEGV/SIGABRT/SIGFPE signal handler** in `main.cpp` with `backtrace_symbols_fd()` — will print a full stack trace on crash
- [x] Added `-rdynamic` linker flag in CMakeLists.txt so backtrace resolves symbol names
- [x] Added guard in `current_disk_index_is_slected()` for empty `m_DiskItemWidgetPtrVec`
- [x] Added guard in `current_network_index_is_slected()` for empty `m_EthItemWidgetPtrVec`

### Files Changed
| File | Change |
|------|--------|
| `main.cpp` | Added crash signal handler + backtrace, granular std::cout prints, screen info |
| `ltm.cpp` | Granular debug prints in ctor, null/empty guards, replaced qDebug with std::cout |
| `ltm_disk.cpp` | Empty vector guard in `current_disk_index_is_slected()` |
| `ltm_network.cpp` | Empty vector guard in `current_network_index_is_slected()` |
| `CMakeLists.txt` | Added `-rdynamic` link option for backtrace symbol resolution |

### Next Steps
1. Build and push → GitHub Actions produces AppImage with debug prints
2. Run on Lubuntu 24.04 → capture stdout/stderr
3. The crash handler will print a **symbolicated backtrace** showing the exact function that segfaulted
4. Based on output, apply the fix (likely the null `item(0)` guard is already in place, but the backtrace will confirm)
5. Once fixed, remove debug instrumentation for release
