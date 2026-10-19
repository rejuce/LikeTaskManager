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

## TODO / Next Steps
- [ ] Create `CMakeLists.txt` replacing `LikeTaskManager.pro`
- [ ] Replace hardcoded Qwt path with `find_package(Qwt)` or `find_package(Qt6Widgets)` + Qwt
- [ ] Add `qt_add_resources()` for `.qrc` files
- [ ] Add `qt6_add_translations()` for `.ts` file
- [ ] Handle `libnethogs` linkage via CMake
- [ ] Remove `Makefile`, `.qmake.stash`, `LikeTaskManager.pro.user`
- [ ] Test build with CMake + Qt6
- [ ] Remove `google867aaac7f0fdc31e.html` (owner verification file, not needed in repo)
