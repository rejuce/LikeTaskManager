#ifndef QDARKSTYLE_H
#define QDARKSTYLE_H

#include <QString>

// Minimal QDarkStyle wrapper that loads the bundled style.qss
// from the Qt resource system (qrc:/qdarkstyle/style.qss).
//
// Original project: https://github.com/ColinDuquesnoy/QDarkStyleSheet
// The full QDarkStyleSheet class embeds the QSS as a C++ string literal.
// Here we read it from the .qrc resource instead, which is simpler and
// avoids a ~100 KB generated header.
// Named QDarkStyleSheet to match the original library API used in main.cpp.
class QDarkStyleSheet
{
public:
    static QString generateQDarkStyleSheet();
};

#endif // QDARKSTYLE_H
