#include "ramitemwidget.h"

RamItemWidget::RamItemWidget(const QString name, QListWidget* view) : PerfItemWidget(name,PlotType::RAM,view,false)
{

}

void RamItemWidget::update_data(double used, double total)
{
    m_firstLineTxt->setText(QString::number(used,'f',1) + "/" + QString::number(total,'f',1) + " GiB (" + QString::number(used/total*100,'f',1) + "%)");
}
