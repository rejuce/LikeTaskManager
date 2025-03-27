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
Ubut 16.04 (for Qt5 Version) or 20.04 for Qt6. No messing with dpendencies, no install, just run.
It can always be updated by clicking in the topbar "Gereral-> UpdateApp" pulling the latest version via deltaupdate using appimageupdate from Github.


## How does it work?
All the stats are collected by using sysfs or Linux Kernel API. For networking stats per process  libnethogs/pcap, the underlying library of the popular tool "nethogs" is used. 
The qt6 version also depends on >qwt6.3, which need to be build from source. Debian packages curretly exist up to 6.1.3 which does not support qt6


## Current state and outlook?  
Right now the core functionality is there. 
There are some additonal static information to be grabbed and a bit more polishing to be done. 
Also dynamically adding and removing disk (for example unplugging USB drive)


In the future it would be nice to additionally gather GPU stats, which is rather hard as there is no gerneral API, same for bluetooth stats. 

If other people find it useful I am also open to feature requests.

If you are feeling generous and like what the tool does for you I am happy about some donations

[![Donate via PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=WQSZJF7HXVF8N)


