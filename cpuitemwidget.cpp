#include "cpuitemwidget.h"

CPUItemWidget::CPUItemWidget(const QString name, QListWidget* view) : PerfItemWidget(name,PlotType::CPU,view,false){


}

void CPUItemWidget::update_data(int pctLoad, double currentGhz)
{
    m_firstLineTxt->setText(QString::number(pctLoad) + "%  " + QString::number(currentGhz,'f',2) + " Ghz");
}


