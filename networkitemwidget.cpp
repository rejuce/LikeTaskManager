#include "networkitemwidget.h"

NetworkItemWidget::NetworkItemWidget(const QString name, QListWidget* view) : PerfItemWidget("Ethernet",PlotType::Ethernet,view,true)
{
m_firstLineTxt->setText(name);

QFont font = m_firstLineTxt->font();
font.setBold(true);
m_firstLineTxt->setFont(font);

set_icon(":/images/ethernet.png");

}

void NetworkItemWidget::update_data(int sendKbs, int recKbs)
{
    static const QString KBs = " KB/s";
    static const QString MBs = " MB/s";
    int& currentRx = recKbs;
    int currentTx = -sendKbs;
    QString currentRXstr = (currentRx<2000) ? QString::number(currentRx,'f',0) + KBs : QString::number(currentRx/1000.0,'f',0) + MBs;
    QString currentTxstr = (currentTx<2000) ? QString::number(currentTx,'f',0) + KBs : QString::number(currentTx/1000.0,'f',0) + MBs;

    m_secondLineTxt->setText("Rx.: " +currentRXstr + "   Tx.: " +currentTxstr);

    //m_secondLineTxt->setText("Rx.: " + QString::number(recKbs,'f',1) + "  Tx.: " + QString::number(-sendKbs,'f',1) + " kbit/s");
}
