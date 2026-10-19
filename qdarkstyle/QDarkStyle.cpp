#include "QDarkStyle.h"

#include <QFile>
#include <QDebug>

QString QDarkStyleSheet::generateQDarkStyleSheet()
{
    QFile file(QStringLiteral(":/qdarkstyle/style.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "QDarkStyle: cannot open qrc:/qdarkstyle/style.qss";
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}
