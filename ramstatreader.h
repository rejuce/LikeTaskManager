#ifndef RAMSTATREADER_H
#define RAMSTATREADER_H

#include <QObject>

class RamStatReader : public QObject
{
    Q_OBJECT
public:
    explicit RamStatReader(QObject *parent = nullptr);

signals:

};

#endif // RAMSTATREADER_H
