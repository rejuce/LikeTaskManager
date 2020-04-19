#ifndef PERFITEMWIDGET_H
#define PERFITEMWIDGET_H

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include <QLabel>

class PerfItemWidget : public QListWidgetItem
{
public:
    PerfItemWidget(const QString name, QListWidget* view, bool isThreeLined);

protected:
    QLabel* m_firstLineTxt= nullptr;
    QLabel* m_secondLineTxt= nullptr;
    QLabel* m_icon = nullptr;
};

#endif // PERFITEMWIDGET_H
