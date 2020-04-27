#ifndef NETWORKITEMWIDGET_H
#define NETWORKITEMWIDGET_H

#include "perfitemwidget.h"

class NetworkItemWidget : public PerfItemWidget
{
public:
    NetworkItemWidget(const QString name, QListWidget* view);
    void update_data(int sendKbs, int recKbs);
};

#endif // NETWORKITEMWIDGET_H
