#ifndef LTM_H
#define LTM_H

#include <QMainWindow>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include "cpuitemwidget.h"
#include <cpustatreader.h>
#include "networkitemwidget.h"
#include "networkstatreader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LTM; }
QT_END_NAMESPACE

class LTM : public QMainWindow
{
    Q_OBJECT

public:
    LTM(QWidget *parent = nullptr);
    ~LTM();

private slots:
    void on_pushButton_clicked();

    void plot_cpu_activity();
    void plot_network_activity();



private:
    Ui::LTM *ui;


    //CPU Stiff
    CPUItemWidget* m_cpuItemWidgetPtr;
    CPUStatReader m_CPUStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridCpuPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataCpuPtrVec;
    void setup_cpu_plots();

    //Network Stuff
    QVector<NetworkItemWidget*> m_cpuItemWidgetPtrVec;
    NetworkStatReader m_NetworkStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridNetworkPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataNetworkPtrVec;
    void setup_network_plots();




    void deselectAllCategories();
    QVector<double> xAchsisBase600;








    //std::vector<std::unique_ptr<QwtCurveFitter>> m_curveFitterPtrVec;




};
#endif // LTM_H
