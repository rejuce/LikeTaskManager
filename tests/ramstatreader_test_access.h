// Test-only access wrapper for RamStatReader.
// Pre-includes Qt headers so that the access trick below only re-protects
// the project headers (which are not yet included at this point).
// The test class in the .cpp file must avoid `private`/`protected` sections.
#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QList>
#include <deque>
#include <thread>
#include <mutex>

#define private public
#define protected public
#include "StatReader.h"
#include "ramstatreader.h"
#undef private
#undef protected
