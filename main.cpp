#include <QApplication>
#include <QScreen>
#include "ltm.h"
#include <iostream>
#include <csignal>
#include <execinfo.h>
#include <cstdlib>
#include <cstdio>

// https://www.particlephysics.sussex.ac.uk/
// https://www.particlephysics.sussex.ac.uk/~peterg/2015-10-07/

//https://github.com/ColinDuquesnoy/QDarkStyleSheet/blob/master/src/QDarkStyleSheet.cpp
//https://github.com/ColinDuquesnoy/QDarkStyleSheet

#include <QDarkStyle.h>

// ---- SIGSEGV / SIGABRT backtrace handler ----
static void crash_handler(int sig)
{
    void* frames[64];
    int n = backtrace(frames, 64);
    fprintf(stderr, "\n=== CRASH: signal %d ===\n", sig);
    backtrace_symbols_fd(frames, n, fileno(stderr));
    fprintf(stderr, "=== END BACKTRACE ===\n");
    // Re-raise with default handler so the core dump still happens
    signal(sig, SIG_DFL);
    raise(sig);
}
// ----------------------------------------------

int main(int argc, char *argv[])
{
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    signal(SIGFPE,  crash_handler);

    std::cout << "[main] setting auto DPI screen scale factor" << std::endl;
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");

    std::cout << "[main] instantiating QApplication" << std::endl;
    QApplication a(argc, argv);
    std::cout << "[main] QApplication created, screen size=" 
              << (a.primaryScreen() ? a.primaryScreen()->size().width() : -1) << "x"
              << (a.primaryScreen() ? a.primaryScreen()->size().height() : -1)
              << " scale factor=" 
              << (a.primaryScreen() ? a.primaryScreen()->devicePixelRatio() : -1.0)
              << std::endl;


    std::cout << "[main] instantiating MainWindow (LTM ctor starts now)" << std::endl;
    LTM w;
    std::cout << "[main] LTM constructor returned" << std::endl;

    std::cout << "[main] loading qDarkStylesheet..." << std::endl;
    a.setStyleSheet(QDarkStyleSheet::generateQDarkStyleSheet());
    std::cout << "[main] qDarkStylesheet loaded" << std::endl;


    std::cout << "[main] calling w.show()" << std::endl;
    w.show();
    std::cout << "[main] w.show() done, entering event loop" << std::endl;

    return a.exec();
}
