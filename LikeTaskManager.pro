QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cpuhelpermethods.cpp \
    cpuitemwidget.cpp \
    cpustatreader.cpp \
    cpustats.cpp \
    diskitemwidget.cpp \
    diskstatreader.cpp \
    ltm_cpu.cpp \
    ltm_disk.cpp \
    ltm_network.cpp \
    ltm_ram.cpp \
    networkitemwidget.cpp \
    networkstatreader.cpp \
perfitemwidget.cpp \
    main.cpp \
    ltm.cpp \
    processstatreader.cpp \
    ramitemwidget.cpp \
    ramstatreader.cpp

HEADERS += \
    StatReader.h \
    cpuhelpermethods.h \
    cpuitemwidget.h \
    cpustatreader.h \
    cpustats.h \
    diskitemwidget.h \
    diskstatreader.h \
    networkitemwidget.h \
    networkstatreader.h \
perfitemwidget.h \
    ltm.h \
    processstatreader.h \
    ramitemwidget.h \
    ramstatreader.h

FORMS += \
    ltm.ui

TRANSLATIONS += \
    LikeTaskManager_de_AT.ts

INCLUDEPATH += /usr/include/qwt
LIBS += -lqwt-qt5

RESOURCES += qdarkstyle/style.qrc
RESOURCES += images.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
