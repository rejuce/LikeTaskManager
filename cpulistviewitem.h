#ifndef CPULISTVIEWITEM_H
#define CPULISTVIEWITEM_H

#include <QObject>
#include <QWidget>
#include <QListWidgetItem>
#include <QLabel>

class CPUListViewItem : public QListWidgetItem
{

public:
    CPUListViewItem(const QString text, QListWidget* view);
    void set_cpu_activity(int val);
    void set_cpu_clock(int val);


private:
    QLabel*  m_cpuActitivty;
    QLabel*  m_clockSpeed;

};

#endif // CPULISTVIEWITEM_H
