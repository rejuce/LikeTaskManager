#include "ltm.h"
#include "ui_ltm.h"
#include <memory>
#include <iostream>
#include "cpustats.h"
#include <QThread>
#include <qwt_curve_fitter.h>
#include "cpuitemwidget.h"
#include <QVector>
#include "cpustatreader.h"
#include <qwt_scale_engine.h>
#include "ramitemwidget.h"


LTM::LTM(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LTM)

{
    ui->setupUi(this);


   // for(size_t i=0; i<cputCnt+1; i++){
   // m_curveDataCpuPtrVec[i]->setCurveFitter(m_curveFitterPtrVec[i].get());
      //  m_curveDataCpuPtrVec[i]->setCurveAttribute(QwtPlotCurve::CurveAttribute::Fitted,true);
  //  }

    setup_cpu_plots();
    setup_network_plots();

    for(double i=0.0; i<600; i++){
        xAchsisBase600.push_back(i);

    }

    if(connect(&m_CPUStatReaderT,&CPUStatReader::data_ready,this,&LTM::plot_cpu_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "connectione stabilsed";

    m_CPUStatReaderT.start();


    if(connect(&m_NetworkStatReaderT,&NetworkStatReader::data_ready,this,&LTM::plot_network_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "connectione stabilsed";

    m_NetworkStatReaderT.start();





//testEntry->set_cpu_clock(20);
 //   ui->listWidget->addItem(rowW);
    m_cpuItemWidgetPtr = new CPUItemWidget("CPU",ui->listWidget);





     m_RamItemWidgetPtr =   new RamItemWidget("RAM",ui->listWidget);

for(int i=0; i<m_NetworkStatReaderT.getDeviceCount(); i++){
    m_EthItemWidgetPtrVec.push_back(new NetworkItemWidget(m_NetworkStatReaderT.getStatData(i).AdapterName,ui->listWidget));

}

//ui->listWidget->setItemWidget( ui->listWidget->item(0), &m_cpuListWidget.widgetO);
ui->listWidget->item(0)->setSelected(true);
//auto testitem = new CPUListWidget();


//ui->listWidget->setItemWidget( ui->listWidget->item(1), &testitem->widgetO);
//ui->listWidget->item(1)->setSizeHint(testitem->widgetO.sizeHint());


//ui->listWidget->item(0)->setFlags(ui->listWidget->item(0)->flags()| Qt::ItemIsSelectable | Qt::ItemIsEnabled);

//ui->listWidget->setItemWidget( ui->listWidget->item(2), label);
//label->setText("boom");
  //  ui->listWidget->addItem(testEntry,ui->listWidget);
 //  ui->listWidget->item(2)->setFlags(ui->listWidget->item(0)->flags());
 //   ui->listWidget->setCurrentRow(2);
//auto ptr = dynamic_cast<CPUListViewItem*>(ui->listWidget->item(2));
//ptr->set_cpu_clock(20);
       auto test =  ui->listWidget->currentRow();
  // dynamic_cast<CPUListViewItem*>(ui->listWidget->currentItem())->set_cpu_clock(99);
     //ui->listWidget->item(2)->setSelected(true);
test++;
}

LTM::~LTM()
{
    delete ui;
}



void LTM::deselectAllCategories()
{

}


void LTM::on_pushButton_clicked(){

    auto cpuptr = dynamic_cast<CPUItemWidget*>(ui->listWidget->item(0));
    cpuptr->update_data(454,2222);
//    while(1){
//        qApp->processEvents();
//        auto currentCPU = CPUStats::get_cpus_activity();
//        static std::vector<QVector<double>> cpuYPlotData(currentCPU.size(),QVector<double>(xAchsisBase600.size(),0));
//        for(size_t i=0; i<currentCPU.size(); i++){
//            cpuYPlotData[i].pop_front();
//            cpuYPlotData[i].push_back(currentCPU[i].second);

//            //filtering first order
//            //for(int k = +1; k < cpuYPlotData[i].size()-1; k++)
//            //int k = cpuYPlotData[i].size()-2;
//           //     cpuYPlotData[i][k] = (cpuYPlotData[i][k-1]+ cpuYPlotData[i][k] +cpuYPlotData[i][k+1] )/ 3.0;

//            //filtering 2nd order to power n
//            int k = cpuYPlotData[i].size()-3;
//            for(int n=8; n>=0; n--){
//            //for(int k = +2; k < cpuYPlotData[i].size()-2; k++)
//            cpuYPlotData[i][k] = (cpuYPlotData[i][k-2] + cpuYPlotData[i][k-1] + cpuYPlotData[i][k] +cpuYPlotData[i][k+1] +cpuYPlotData[i][k+2] )/ 5.0;
//            k--;
//            }




//            if(i>0){

//                m_curveDataCpuPtrVec[i]->setZ(0);
//            }

//                else{
//                static int slowCnter = 0;
//                if(slowCnter%5==0)     m_cpuItemWidgetPtr->update_data(cpuYPlotData[i][cpuYPlotData[i].size()-1],4);
//                //auto val = cpuYPlotData[i][cpuYPlotData[i].size()-1];

//               // m_cpuListWidget.m_clockSpeed->setNum(val);
//                m_curveDataCpuPtrVec[0]->setZ(2);
//                slowCnter++;
//            }

//            m_curveDataCpuPtrVec[i]->setSamples(xAchsisBase600,cpuYPlotData[i]);

//        }
//        QThread::msleep(100);
//        ui->plotCpu->replot();

//    }

}




void LTM::on_listWidget_itemClicked(QListWidgetItem *item){
    auto perfItemWidetPtr = dynamic_cast<PerfItemWidget*>(item);

    switch(perfItemWidetPtr->getPlotType()){
    case PlotType::CPU : ui->stackedWidget->setCurrentIndex(0); break;
    case PlotType::RAM : ui->stackedWidget->setCurrentIndex(1); break;
    case PlotType::DISK : ui->stackedWidget->setCurrentIndex(2); break;
    case PlotType::Ethernet : ui->stackedWidget->setCurrentIndex(3); break;
    case PlotType::WIFI : ui->stackedWidget->setCurrentIndex(4); break;
    case PlotType::GPU : ui->stackedWidget->setCurrentIndex(5); break;
    }


}
