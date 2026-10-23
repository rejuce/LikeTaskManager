# LikeTaskManager — Project Outline

Qt6 system monitor app (CPU / RAM / disk / network / processes), packaged as
AppImage with built-in autoupdate.

## Key files

| Path | Purpose |
|------|---------|
| `main.cpp` | Entry point, crash handler, QDarkStyle load |
| `ltm.h` / `ltm.cpp` | MainWindow; **built-in autoupdate** (runs `usr/bin/appimageupdatetool-x86_64.AppImage` from `applicationDirPath()`, lines ~253–262) |
| `ltm_cpu.cpp`, `ltm_ram.cpp`, `ltm_disk.cpp`, `ltm_network.cpp` | Per-resource pages |
| `processstatreader.{h,cpp}` | `/proc` process list reader |
| `processsorttree.{h,cpp}` | QTreeWidget subclass: numeric sort + expansion preservation |
| `CMakeLists.txt` | Build (CMake + Ninja), test targets |
| `tests/` | Unit tests (ctest, ~9 suites / 73 cases) |
| `.github/workflows/build-appimage.yml` | CI: build Qwt + libnethogs, AppImage bundle, smoke test, release publish on `v*` tags |

## AppImage autoupdate chain

1. **Embedded URL** (set by `-updateinformation` in the workflow):
   `zsync|https://github.com/rejuce/LikeTaskManager/releases/latest/download/LikeTaskManager-x86_64.AppImage.zsync`
2. **`appimageupdatetool`** (bundled at `AppDir/usr/bin/appimageupdatetool-x86_64.AppImage`)
   reads the embedded URL → invokes `zsync2` from the AppImage runtime.
3. **zsync2** downloads the `.zsync` from `releases/latest` (GitHub 302s to a
   short-lived signed CDN URL — normal), then fetches only the changed
   2048-byte blocks of `LikeTaskManager-x86_64.AppImage` (relative URL inside
   the `.zsync`) and patches the running AppImage.
4. **Fixed asset name** (`LikeTaskManager-x86_64.AppImage` + `mv` guard in CI)
   keeps the update URL stable across versions; `releases/latest` always
   points at the newest release → autoupdate tracks the latest release.

## Status snapshot (2026-10-23)

- Latest release: **v4.0.3** (2026-10-22)
- Autoupdate verified working end-to-end (user-confirmed).
- Open: workflow fix bundling `appimageupdatetool` into AppDir is committed
  locally on `master`, **not pushed** (awaiting approval).
- Full history: see `progress.md`.
