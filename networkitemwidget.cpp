#include "networkitemwidget.h"

NetworkItemWidget::NetworkItemWidget(const QString name, QListWidget* view) : PerfItemWidget(name,view,true)
{



}

void NetworkItemWidget::update_data(int sendKbs, int recKbs)
{
    m_secondLineTxt->setText("Rx: " + QString::number(recKbs,'f',2) + "  Tx:" + QString::number(sendKbs,'f',2) + " kbit/s");
}
