#include "cpulistwidget.h"
#include <QGridLayout>

CPUListWidget::CPUListWidget()
{
     auto widget = &widgetO;
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
    widget->setStyleSheet("background-color: transparent;}"
                          "QWidget:hover {   outline: 0; background-color: #32414B;}"
                          "QWidget:selected      {   outline: 0; background-color:  #1464A0;}"
                          "QWidget:!active {   outline: 0; background-color:  transparent;}"



                          );

    //setSizeHint(widget->sizeHint());

   // this->setFlags(flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
   // connect(widget, &QWidget::sel)
}


void CPUListWidget::set_cpu_activity(int val)
{
    m_cpuActitivty->setText(QString::number(val) + "%" );
}
