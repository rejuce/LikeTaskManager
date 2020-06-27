#ifndef DISKITEMWIDGET_H
#define DISKITEMWIDGET_H

#include "perfitemwidget.h"

class DiskItemWidget : public PerfItemWidget
{
public:
    DiskItemWidget(const QString type,const QString name, QListWidget* view);
    void update_data(QString& read, QString& write,  QString&  active);
};

#endif // DISKITEMWIDGET_H
