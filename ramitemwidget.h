#ifndef RAMITEMWIDGET_H
#define RAMITEMWIDGET_H

#include "perfitemwidget.h"

class RamItemWidget : public PerfItemWidget
{

public:
    RamItemWidget(const QString name, QListWidget* view);
    void update_data(double used, double total);
};

#endif // RAMITEMWIDGET_H
