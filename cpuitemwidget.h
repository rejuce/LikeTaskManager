#ifndef CPUITEMWIDGET_H
#define CPUITEMWIDGET_H

#include "perfitemwidget.h"

class CPUItemWidget : public PerfItemWidget
{
public:
    CPUItemWidget(const QString name, QListWidget* view);
    void update_data(int pctLoad, double currentGhz);
};

#endif // CPUITEMWIDGET_H
