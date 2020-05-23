## What is "LikeTaskManager"?
A Qt based, self-contained (AppImage) process monitor that mimics the feel 
and functionality of Windows10 Task Manager's performance tab under Linux.

![Alt text](ltm.PNG?raw=true "UI Preview")


## Why does the Linux world need another performance monitor? 

There are a myriad of very specialised tools to monitor very specific stats in great detail. But I found none
with the simplicity and compactness of windows task manager, while still providing a good overview of various system
performance indicators while not being forced to stare at a terminal or eyesoring terminal graphing. 
For example gnome performance monitor looks nice, but lacks stats about disk I/O or traffic per network interface. Glances 
gives a very comprehensive overview but is hard to keep a good overview.


## How to Install?  

Just download appimage from release tab, give execute permission and thats it. Should work on all Linux flavours newer than 
Ubut 16.04. No messing with dpendencies, no install, just run.
It can always be updated by clicking in the topbar "Gereral-> UpdateApp" pulling the latest version via deltaupdate using appimageupdate from Github.


## How does it work?
All the stats are collected by using sysfs  or Linux Kernel API. None of the stats so far are collected by calling third party tools.


## Current state and outlook?  
Right now the core functionality is there. 
There are some additonal static information to be grabbed and a bit more polishing to be done. 
Also dynamically adding and removing disk (for example unplugging USB drive) will be added soon.
Support for QHD, 4K Monitors will be added soon

In the future it would be nice to additionally gather GPU stats, which is rather hard as there is no gerneral API, same for bluetooth stats. Also a listing per process is an option.

If other people find it usefull I am also open to feature requests.
