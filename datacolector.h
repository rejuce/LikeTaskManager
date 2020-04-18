#ifndef DATACOLECTOR_H
#define DATACOLECTOR_H

#include <QObject>
#include <QThread>

class DataColector : public QThread
{
public:
    DataColector();
};

#endif // DATACOLECTOR_H
