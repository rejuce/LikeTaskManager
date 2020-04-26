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



private:
    CPUItemWidget* m_cpuItemWidgetPtr;

    void setup_cpu_plots();


    void deselectAllCategories();
    QVector<double> xAchsisBase600;

    CPUStatReader CPUStatReaderT;


    std::unique_ptr<QwtPlotGrid> m_gridCpuPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataCpuPtrVec;
    //std::vector<std::unique_ptr<QwtCurveFitter>> m_curveFitterPtrVec;
    Ui::LTM *ui;



};
#endif // LTM_H
