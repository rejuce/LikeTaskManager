#ifndef PERFITEMWIDGET_H
#define PERFITEMWIDGET_H

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include <QLabel>

enum struct PlotType{
    CPU,
    RAM,
    DISK,
    Ethernet,
    WIFI,
    GPU
};

class PerfItemWidget : public QListWidgetItem
{
public:
    PerfItemWidget(const QString name, PlotType isPlotOn, QListWidget* view, bool isThreeLined);
    PlotType getPlotType(){return m_isPlottedOn;}

protected:
    QLabel* m_firstLineTxt= nullptr;
    QLabel* m_secondLineTxt= nullptr;
    QLabel* m_icon = nullptr;
    const PlotType m_isPlottedOn;
};

#endif // PERFITEMWIDGET_H
