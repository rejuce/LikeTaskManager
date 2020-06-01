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
#include "ramitemwidget.h"
#include "ramstatreader.h"
#include "diskstatreader.h"
#include "diskitemwidget.h"
#include "processstatreader.h"
#include <QProcess>


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

    void plot_ram_activity();

    void plot_network_activity();
    void update_static_network_info_from(StatTypes::NetworkData& networkData);

    void plot_disk_activity();

    void handle_process_data();

    void selfUpdate();
    void printInfo();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_butUpdateOK_clicked();

private:
    Ui::LTM *ui;


    //CPU Stiff
    CPUItemWidget* m_cpuItemWidgetPtr;
    CPUStatReader m_CPUStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridCpuPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataCpuPtrVec;
    void setup_cpu_plots();

    //RAM Sutff
    RamItemWidget* m_RamItemWidgetPtr;
    RamStatReader m_RamStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridRamPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataRamPtrVec;
    void setup_ram_plots();


    //Disk Stuff
    QVector<DiskItemWidget*> m_DiskItemWidgetPtrVec;
    DiskStatReader m_DiskStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridDiskPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataDiskPtrVec;
    void setup_disk_plots();
    bool current_disk_index_is_slected(int diskindex);  //!> returns true,

    //Network Stuff
    QVector<NetworkItemWidget*> m_EthItemWidgetPtrVec;
    NetworkStatReader m_NetworkStatReaderT;
    std::unique_ptr<QwtPlotGrid> m_gridNetworkPtr;
    std::vector<std::unique_ptr<QwtPlotCurve>> m_curveDataNetworkPtrVec;
    void setup_network_plots();
    bool current_network_index_is_slected(int networkindex);  //!> returns true, if networkindex 0...nth Network is selected in the left list WIdget


    ProcessStatReader m_ProcessStatReaderT;



    void deselectAllCategories();
    QVector<double> xAchsisBase600;


QProcess* updateprocess;





    //std::vector<std::unique_ptr<QwtCurveFitter>> m_curveFitterPtrVec;




};
#endif // LTM_H
