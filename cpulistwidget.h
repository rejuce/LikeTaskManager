#ifndef CPULISTWIDGET_H
#define CPULISTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class CPUListWidget
{
public:
    CPUListWidget();

    void set_cpu_activity(int val);
    void set_cpu_clock(int val);

    QWidget widgetO;
    QLabel*  m_cpuActitivty;
    QLabel*  m_clockSpeed;

};

#endif // CPULISTWIDGET_H
