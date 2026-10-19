## What is "LikeTaskManager"?
A Qt based, self-contained (AppImage) process monitor that mimics the feel 
and functionality of Windows 10 Task Manager's performance and process tab under Linux.
It enables to oversee sytemwide CPU, RAM, Disk I/O and Network untilisation per device and per process.

![Alt text](ltm2.PNG?raw=true "UI Preview")

now in Version 2.0 additionally CPU Load, Memory, Disk I/O and with root permission network I/O per process can be diskplayed

![Alt text](ltm3.PNG?raw=true "UI Process list Preview")


## Why does the Linux world need another performance monitor? 

There are a myriad of very specialised tools to monitor very specific stats in great detail. But I found none
with the simplicity and compactness of windows task manager, while still providing a good overview of various system
performance indicators while not being forced to stare at a terminal or eyesoring terminal graphing. 
For example gnome performance monitor looks nice, but lacks stats about disk I/O or traffic per network interface. Glances 
gives a very comprehensive overview but is hard to keep a good overview.


## How to Install?  

Just download appimage from release tab, give execute permission and thats it. Should work on all Linux flavours equal or newer than 
Ubut 16.04. No messing with dpendencies, no install, just run.
It can always be updated by clicking in the topbar "Gereral-> UpdateApp" pulling the latest version via deltaupdate using appimageupdate from Github.


## Build from Source

The project is built with **CMake** (>= 3.16) and **C++17**. Tested on Debian 13 / Ubuntu 22.04 with Qt 6.

### 1. Install the Qt 6 toolchain and build tools (apt)

```bash
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    qt6-base-dev libqt6network6 libqt6widgets6 qt6-svg-dev qt6-l10n-tools \
    cmake ninja-build meson make g++ git patchelf \
    libnl-3-dev libnl-genl-3-dev libpcap-dev
```

> `qt6-svg-dev` is required because Qwt's `qwt_symbol.cpp` includes `<qsvgrenderer.h>`.
> `libnl-3-dev`, `libnl-genl-3-dev` and `libpcap-dev` are needed to build libnethogs.
> `patchelf` is needed to fix libnethogs' broken SONAME (see step 3).

### 2. Build & install Qwt 6.3.0

Qwt 6.3.0 is **not** packaged for Qt 6 on Debian (only the Qt 5 variant `libqwt-qt5-dev` exists) and its GitHub repo (`KDE/qwt`) no longer exists, so the source is fetched from **SourceForge**:

```bash
mkdir -p deps-src && cd deps-src
curl -sL -o qwt-6.3.0.tar.bz2 \
    "https://sourceforge.net/projects/qwt/files/qwt/6.3.0/qwt-6.3.0.tar.bz2/download"
tar xjf qwt-6.3.0.tar.bz2
cd qwt-6.3.0
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_DOCS=OFF
cmake --build build
sudo cmake --install build
sudo ldconfig
```

Qwt ships no CMake config package, so the project locates it via `find_path`/`find_library`.
If it is installed to a non-default prefix, pass it as `-DQWT_ROOT=<prefix>`.

### 3. Build & install libnethogs 0.8.8

Per-process network stats rely on **libnethogs** (the library behind the `nethogs` tool), which is not packaged as a `-dev` library on Debian. Build it from source with meson/ninja:

```bash
cd deps-src
curl -sL -o nethogs-0.8.8.tar.gz \
    "https://github.com/ntop/nethogs/archive/refs/tags/v0.8.8.tar.gz"
tar xzf nethogs-0.8.8.tar.gz
cd nethogs-0.8.8
meson setup build -Ddisable-wifi=true -Ddisable-nvidia=true
meson compile -C build
sudo meson install -C build
```

**Note on naming:** the library that gets installed is `liblibnethogs.so`, and its meson build
bakes in the SONAME `liblibnethogs.so.undefined`. This project links against `-lnethogs` and
includes `<libnethogs.h>`. Fix the SONAME and create two compatibility links:

```bash
sudo patchelf --set-soname liblibnethogs.so /usr/local/lib/x86_64-linux-gnu/liblibnethogs.so
sudo ln -sf /usr/local/lib/x86_64-linux-gnu/liblibnethogs.so /usr/local/lib/x86_64-linux-gnu/libnethogs.so
sudo ln -sf /usr/local/include/libnethogs/libnethogs.h /usr/local/include/libnethogs.h
sudo ldconfig
```

### 4. Build LikeTaskManager (CMake)

Build out-of-tree so the source directory stays clean:

```bash
cmake -S /path/to/LikeTaskManager -B /tmp/ltm-build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/ltm-build
```

The resulting executable is `LikeTaskManager` in the build directory.

### 5. Run the unit tests

```bash
cd /tmp/ltm-build
ctest --output-on-failure
```

### 6. Run

```bash
./LikeTaskManager
```

> On a system without a display (e.g. a container), run headless with `QT_QPA_PLATFORM=offscreen ./LikeTaskManager`.
> **Per-process network I/O requires root / `CAP_NET_ADMIN`.** Without it, the nethogs loop fails gracefully (`Operation not permitted`) and the app continues to show CPU, RAM, disk and system-wide network stats.

### Building the AppImage

The AppImage is built automatically by the **GitHub Actions** workflow (`.github/workflows/build-appimage.yml`)
on every push to `qt6` / `Qt6Cmake` — the resulting `LikeTaskManager-x86_64.AppImage` is available
as a workflow artifact. It builds on `ubuntu-22.04` (glibc 2.35, the newest version linuxdeployqt supports),
compiles Qwt and libnethogs from source, bundles everything with `linuxdeployqt`, runs a headless
smoke test, and uploads the AppImage.


## How does it work?
All the stats are collected by using sysfs or Linux Kernel API. For networking stats per process  libnethogs/pcap, the underlying library of the popular tool "nethogs" is used. 


## Current state and outlook?  
Right now the core functionality is there. 
There are some additonal static information to be grabbed and a bit more polishing to be done. 
Also dynamically adding and removing disk (for example unplugging USB drive) will be added soon.


In the future it would be nice to additionally gather GPU stats, which is rather hard as there is no gerneral API, same for bluetooth stats. 

If other people find it usefull I am also open to feature requests.
