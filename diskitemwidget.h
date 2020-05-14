#ifndef DISKITEMWIDGET_H
#define DISKITEMWIDGET_H

#include "perfitemwidget.h"

class DiskItemWidget : public PerfItemWidget
{
public:
    DiskItemWidget(const QString name, QListWidget* view);
    void update_data(double activity, double io);
};

#endif // DISKITEMWIDGET_H
