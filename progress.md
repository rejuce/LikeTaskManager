# LikeTaskManager — Progress

## Current Focus: Debugging Segfault on Lubuntu 24.04 (headless/VNC)

### Status: Debug instrumentation added, ready to build & test

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
