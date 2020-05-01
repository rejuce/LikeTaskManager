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





       new CPUItemWidget("RAM",ui->listWidget);

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

void LTM::setup_cpu_plots()
{

    m_gridCpuPtr = std::make_unique<QwtPlotGrid>();
    size_t cputCnt = CPUStats::get_cpu_count();

    for(size_t i=0; i<cputCnt+1; i++){//+ for sum of all
         m_curveDataCpuPtrVec.emplace_back(std::make_unique<QwtPlotCurve>());
      //   m_curveFitterPtrVec.emplace_back(std::make_unique<QwtSplineCurveFitter>());

    }
   // ui->plotCpu->setTitle("");
   // ui->plotCpu->setCanvasBackground( Qt::white );
    ui->plotCpu->enableAxis( QwtPlot::yRight,true );
    ui->plotCpu->enableAxis( QwtPlot::yLeft,false );
    ui->plotCpu->enableAxis( QwtPlot::xBottom,false );
    ui->plotCpu->setAxisAutoScale ( QwtPlot::yRight, false );
    //ui->plotCpu->setAxisAutoScale ( QwtPlot::xBottom, false );
    ui->plotCpu->setAxisScale ( QwtPlot::yRight, 0,100 );
    ui->plotCpu->setAxisTitle(QwtPlot::yRight,"Acitivity %");

    m_gridCpuPtr->setPen(QColor::fromRgb(0xEF,0xF0,0xF1));
    m_gridCpuPtr->attach(ui->plotCpu);


    auto get_rnd_color_vec =  [](int count)->QVector<QColor> {
        QVector<QColor> colors;
        float currentHue = 0.0;
        for (int i = 0; i < count; i++){
            colors.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
            currentHue += 0.618033988749895f;
            currentHue = std::fmod(currentHue, 1.0f);
        }
        return colors;
    };

auto colovec = get_rnd_color_vec(m_curveDataCpuPtrVec.size());
int colorindex=0;
    for(auto& singleCPUCurve : m_curveDataCpuPtrVec){

        singleCPUCurve->attach(ui->plotCpu);
        singleCPUCurve->setPen(colovec[colorindex], 1.5 );
       // singleCPUCurve->setStyle( QwtPlotCurve::CurveStyle::Dots);
        singleCPUCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
        singleCPUCurve->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
        colorindex++;
    }
m_curveDataCpuPtrVec[0]->setPen(Qt::white, 4);//cpu sum

    ui->plotCpu->replot();


}

void LTM::setup_network_plots(){
m_gridNetworkPtr = std::make_unique<QwtPlotGrid>();

size_t NetworkCnt = m_NetworkStatReaderT.getDeviceCount();

for(size_t i=0; i<2; i++){ //one curve for each datadeque in StatTypes::NetworkData, one plot for all network interfaces
     m_curveDataNetworkPtrVec.emplace_back(std::make_unique<QwtPlotCurve>());
  //   m_curveFitterPtrVec.emplace_back(std::make_unique<QwtSplineCurveFitter>());

}
// ui->plotCpu->setTitle("");
// ui->plotCpu->setCanvasBackground( Qt::white );
ui->plotNetwork->enableAxis( QwtPlot::yRight,true );
ui->plotNetwork->enableAxis( QwtPlot::yLeft,false );
ui->plotNetwork->enableAxis( QwtPlot::xBottom,false );
ui->plotNetwork->setAxisAutoScale ( QwtPlot::yRight, true );
ui->plotNetwork->axisScaleEngine(QwtPlot::yRight)->setAttribute(QwtScaleEngine::Attribute::Symmetric, false);
ui->plotNetwork->axisScaleEngine(QwtPlot::yRight)->setMargins(1,1);
//ui->plotCpu->setAxisAutoScale ( QwtPlot::xBottom, false );
//ui->plotCpu->setAxisScale ( QwtPlot::yRight, 0,100 );
ui->plotNetwork->setAxisTitle(QwtPlot::yRight,"KB/s");

m_gridNetworkPtr->setPen(QColor::fromRgb(0xEF,0xF0,0xF1));
m_gridNetworkPtr->enableYMin(false);
m_gridNetworkPtr->setYAxis(QwtPlot::yRight);
m_gridNetworkPtr->attach(ui->plotNetwork);


//auto get_rnd_color_vec =  [](int count)->QVector<QColor> {
//    QVector<QColor> colors;
//    float currentHue = 0.0;
//    for (int i = 0; i < count; i++){
//        colors.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
//        currentHue += 0.618033988749895f;
//        currentHue = std::fmod(currentHue, 1.0f);
//    }
//    return colors;
//};

//auto colovec = get_rnd_color_vec(m_curveDataCpuPtrVec.size());
//int colorindex=0;
for(auto& singleNetworkDataCurve : m_curveDataNetworkPtrVec){
    singleNetworkDataCurve->attach(ui->plotNetwork);
   // singleCPUCurve->setStyle( QwtPlotCurve::CurveStyle::Dots);
    singleNetworkDataCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    singleNetworkDataCurve->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
}

    m_curveDataNetworkPtrVec[0]->setPen(Qt::green, 3 );
       m_curveDataNetworkPtrVec[1]->setPen(Qt::red, 3);



ui->plotNetwork->replot();


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

void LTM::plot_cpu_activity(){

    auto& DataVec = m_CPUStatReaderT.m_DataVec;

    for(size_t i=0; i<DataVec.size(); i++){

       std::lock_guard<std::mutex> lock(m_CPUStatReaderT.m_DataVecMutex);
       if(i>0){

            m_curveDataCpuPtrVec[i]->setZ(0);
        }

            else{
            static int slowCnter = 0;
            if(slowCnter%5==0)     m_cpuItemWidgetPtr->update_data(DataVec[i].currentActivityData[DataVec[i].currentActivityData.size()-1],4);
            //auto val = cpuYPlotData[i][cpuYPlotData[i].size()-1];

           // m_cpuListWidget.m_clockSpeed->setNum(val);
            m_curveDataCpuPtrVec[0]->setZ(2);
            slowCnter++;
        }

        QVector<double> tmpY(DataVec[i].currentActivityData.begin(),DataVec[i].currentActivityData.end());
        m_curveDataCpuPtrVec[i]->setSamples(xAchsisBase600,tmpY);

    }

    ui->plotCpu->replot();

}

void LTM::plot_network_activity()
{
    auto& DataVec = m_NetworkStatReaderT.m_DataVec;

   // for(size_t i=0; i<DataVec.size(); i++){
static int slowCnter = 0;
    for(int i=0; i<DataVec.size(); i++){

       std::lock_guard<std::mutex> lock(m_NetworkStatReaderT.m_DataVecMutex);


       if(slowCnter%5==0) m_EthItemWidgetPtrVec[i]->update_data(DataVec[i].currentTxData.back(), DataVec[i].currentRxData.back());



        QVector<double> tmpRxY(DataVec[i].currentRxData.begin(),DataVec[i].currentRxData.end()-8);
        QVector<double> tmpTxY(DataVec[i].currentTxData.begin(),DataVec[i].currentTxData.end()-8);
        m_curveDataNetworkPtrVec[0]->setSamples(xAchsisBase600,tmpRxY);
        m_curveDataNetworkPtrVec[1]->setSamples(xAchsisBase600,tmpTxY);

    }
       slowCnter++;


   // }

    ui->plotNetwork->replot();
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
