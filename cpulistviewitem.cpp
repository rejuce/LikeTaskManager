#include "cpulistviewitem.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

CPUListViewItem::CPUListViewItem(const QString text, QListWidget* view) :
    QListWidgetItem(view)
{
    QWidget* widget = new QWidget;
    QGridLayout* layout = new QGridLayout(widget);

    auto cpuLabel = new QLabel("CPU",widget);
    cpuLabel->setStyleSheet("background-color: rgba(191, 64, 64, 0);  ");
    layout->addWidget(cpuLabel, 0, 0);


    //layout->addWidget(new QPushButton(text), 0, 1);

    m_cpuActitivty = new QLabel("5%",widget);
    layout->addWidget(m_cpuActitivty, 1, 0);
    m_clockSpeed = new QLabel("5%",widget);
    layout->addWidget(m_clockSpeed, 1, 1);

    widget->setLayout(layout);
    widget->setStyleSheet("QWidget:hover {   outline: 0; background-color: #32414B;}");

    this->setFlags(flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);


   // widget->setStyleSheet("background-color: rgb(173, 127, 168);");
    view->setItemWidget(this, widget);

    setSizeHint(widget->sizeHint());
}




