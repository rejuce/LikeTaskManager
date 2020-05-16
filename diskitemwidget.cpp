#include "diskitemwidget.h"

DiskItemWidget::DiskItemWidget(const QString type, const QString name, QListWidget* view) : PerfItemWidget(type,PlotType::DISK,view,true)
{

    m_firstLineTxt->setText(name);

    QFont font = m_firstLineTxt->font();
    font.setBold(true);
    m_firstLineTxt->setFont(font);

}
