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





void LTM::setup_disk_plots(){
    m_gridDiskPtr = std::make_unique<QwtPlotGrid>();

    size_t DiskCnt = m_DiskStatReaderT.getDeviceCount();

    for(size_t i=0; i<3; i++){ //one curve for each datadeque in StatTypes::DiskData, one plot for all Disk interfaces
        m_curveDataDiskPtrVec.emplace_back(std::make_unique<QwtPlotCurve>());

    }
    // ui->plotCpu->setTitle("");
    // ui->plotCpu->setCanvasBackground( Qt::white );
    ui->plotDisk->enableAxis( QwtPlot::yRight,true );
    ui->plotDisk->enableAxis( QwtPlot::yLeft,true );
    ui->plotDisk->enableAxis( QwtPlot::xBottom,false );
    ui->plotDisk->setAxisAutoScale ( QwtPlot::yRight, true );
    ui->plotDisk->axisScaleEngine(QwtPlot::yRight)->setAttribute(QwtScaleEngine::Attribute::Symmetric, true);
    ui->plotDisk->axisScaleEngine(QwtPlot::yRight)->setMargins(1,1);
    ui->plotDisk->setAxisTitle(QwtPlot::yRight,"KB/s");


    ui->plotCpu->setAxisAutoScale ( QwtPlot::yLeft, false );
    ui->plotCpu->setAxisScale ( QwtPlot::yLeft, 0,100 );
    ui->plotDisk->setAxisTitle(QwtPlot::yLeft,"Activity %");


    m_gridDiskPtr->setPen(QColor::fromRgb(0xEF,0xF0,0xF1));
    m_gridDiskPtr->enableYMin(false);
    m_gridDiskPtr->setYAxis(QwtPlot::yLeft);
    m_gridDiskPtr->attach(ui->plotDisk);


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
    for(auto& singleDiskDataCurve : m_curveDataDiskPtrVec){
        singleDiskDataCurve->attach(ui->plotDisk);
        // singleCPUCurve->setStyle( QwtPlotCurve::CurveStyle::Dots);
        singleDiskDataCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
        singleDiskDataCurve->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
    }

    //override for activity
    m_curveDataDiskPtrVec[2]->setAxes(QwtPlot::xBottom,QwtPlot::yLeft);

    m_curveDataDiskPtrVec[0]->setPen(Qt::green, 3 );
    m_curveDataDiskPtrVec[1]->setPen(Qt::red, 3);
    m_curveDataDiskPtrVec[2]->setPen(Qt::white, 3);



    ui->plotDisk->replot();


}

bool LTM::current_disk_index_is_slected(int index)
{

    int startIndexOfDiskWidgetItems = ui->listWidget->row(dynamic_cast<QListWidgetItem*>(m_DiskItemWidgetPtrVec[0]));
    int currentRow =  ui->listWidget->currentRow();

    //only if current selected corresnposds to current data index plot and update register
    if(currentRow>=startIndexOfDiskWidgetItems &&
            currentRow< startIndexOfDiskWidgetItems+m_DiskItemWidgetPtrVec.size() &&
            index == currentRow-startIndexOfDiskWidgetItems){
        return true;
    }
    else return false;
}






void LTM::plot_disk_activity()
{
    auto& DataVec = m_DiskStatReaderT.m_DataVec;

    // for(size_t i=0; i<DataVec.size(); i++){
    static int slowCnter = 0;
    for(int i=0; i<DataVec.size(); i++){

        std::lock_guard<std::mutex> lock(m_DiskStatReaderT.m_DataVecMutex);


      //  if(slowCnter%m_DiskStatReaderT.m_widgetDataModulus==0) {
       //     m_DiskItemWidgetPtrVec[i]->update_data(DataVec[i].currentTxData.back(), DataVec[i].currentRxData.back());
      //  }




//        //only if current selected corresnposds to current data index plot and update register
//        if(current_disk_index_is_slected(i)){

//            if(slowCnter%m_DiskStatReaderT.m_widgetDataModulus==0) {
//                static const QString KBs = " KB/s";
//                static const QString MBs = " MB/s";
//                double currentRx = *(DataVec[i].currentRxData.end()-8);
//                QString currentRXstr = (currentRx<2000) ? QString::number(currentRx,'f',1) + KBs : QString::number(currentRx/1000.0,'f',1) + MBs;
//                double currentTx = -(*(DataVec[i].currentTxData.end()-8));
//                QString currentTxstr = (currentTx<2000) ? QString::number(currentTx,'f',1) + KBs : QString::number(currentTx/1000.0,'f',1) + MBs;


//                ui->labRecKbs->setText(currentRXstr) ;
//                ui->labSendKbs->setText(currentTxstr) ;
//            }


//            if(slowCnter%m_DiskStatReaderT.m_widgetDataModulus==0) {
//                ui->labNetName->setText(DataVec[i].AdapterName);
//                ui->labNetwIp4->setText(DataVec[i].ip4Addresses.join(", "));
//                ui->labNetwIp6->setText(DataVec[i].ip6Addresses.join(", "));
//                ui->labNetwMac->setText(DataVec[i].hwAddress);

//                switch(DataVec[i].interfaceType){
//                case QdiskInterface::InterfaceType::Ethernet : ui->labNetwType->setText("Ethernet"); break;
//                case QdiskInterface::InterfaceType::Wifi : ui->labNetwType->setText("Wifi");break;
//                case QdiskInterface::InterfaceType::Virtual : ui->labNetwType->setText("Virtual");break;
//                case QdiskInterface::InterfaceType::Loopback : ui->labNetwType->setText("Loopback");break;
//                case QdiskInterface::InterfaceType::CanBus : ui->labNetwType->setText("CAN");break;
//                default : ui->labNetwType->setText("Unknown");break;
//                }


//                //todo ui->labNetwDNS
//                ui->labNetwSpeed->setText(DataVec[i].speedLinkInfo);

//            }



//            QVector<double> tmpRxY(DataVec[i].currentRxData.begin(),DataVec[i].currentRxData.end()-8);
//            QVector<double> tmpTxY(DataVec[i].currentTxData.begin(),DataVec[i].currentTxData.end()-8);
//            m_curveDatadiskPtrVec[0]->setSamples(xAchsisBase600,tmpRxY);
//            m_curveDatadiskPtrVec[1]->setSamples(xAchsisBase600,tmpTxY);

//        }

    }
//    slowCnter++;


    // }

    ui->plotDisk->replot();
}

//void LTM::update_static_disk_info_from(StatTypes::DiskData& diskData){
//    // ui->labName =

//}




