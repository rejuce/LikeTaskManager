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

bool LTM::current_network_index_is_slected(int index)
{

    int startIndexOfEthernetWidgetItems = ui->listWidget->row(dynamic_cast<QListWidgetItem*>(m_EthItemWidgetPtrVec[0]));
    int currentRow =  ui->listWidget->currentRow();

    //only if current selected corresnposds to current data index plot and update register
     return (currentRow>=startIndexOfEthernetWidgetItems &&
            currentRow< startIndexOfEthernetWidgetItems+m_EthItemWidgetPtrVec.size() &&
            index == currentRow-startIndexOfEthernetWidgetItems);
}






void LTM::plot_network_activity()
{

    try{

        auto& DataVec = m_NetworkStatReaderT.m_DataVec;

        // for(size_t i=0; i<DataVec.size(); i++){
        static int slowCnter = 0;
        for(int i=0; i<DataVec.size(); i++){

            std::lock_guard<std::mutex> lock(m_NetworkStatReaderT.m_DataVecMutex);


            if(slowCnter%m_NetworkStatReaderT.m_widgetDataModulus==0) {
                m_EthItemWidgetPtrVec[i]->update_data(DataVec[i].currentTxData.back(), DataVec[i].currentRxData.back());
            }




            //only if current selected corresnposds to current data index plot and update register
            if(current_network_index_is_slected(i)){

                if(slowCnter%NetworkStatReader::m_widgetDataModulus==0) {
                    static const QString KBs = " KB/s";
                    static const QString MBs = " MB/s";
                    constexpr int smoothBackMargin = 8;
                    double currentRx = *(DataVec[i].currentRxData.end()-smoothBackMargin);
                    QString currentRXstr = (currentRx<2000) ? QString::number(currentRx,'f',1) + KBs : QString::number(currentRx/1000.0,'f',1) + MBs;
                    double currentTx = -(*(DataVec[i].currentTxData.end()-smoothBackMargin));
                    QString currentTxstr = (currentTx<2000) ? QString::number(currentTx,'f',1) + KBs : QString::number(currentTx/1000.0,'f',1) + MBs;


                    ui->labRecKbs->setText(currentRXstr) ;
                    ui->labSendKbs->setText(currentTxstr) ;
                }


                if(slowCnter%m_NetworkStatReaderT.m_widgetDataModulus==0) {
                    ui->labNetName->setText(DataVec[i].AdapterName);
                    ui->labNetwIp4->setText(DataVec[i].ip4Addresses.join(", "));
                    ui->labNetwIp6->setText(DataVec[i].ip6Addresses.join(", "));
                    ui->labNetwMac->setText(DataVec[i].hwAddress);

                    switch(DataVec[i].interfaceType){
                    case QNetworkInterface::InterfaceType::Ethernet : ui->labNetwType->setText("Ethernet"); break;
                    case QNetworkInterface::InterfaceType::Wifi : ui->labNetwType->setText("Wifi");break;
                    case QNetworkInterface::InterfaceType::Virtual : ui->labNetwType->setText("Virtual");break;
                    case QNetworkInterface::InterfaceType::Loopback : ui->labNetwType->setText("Loopback");break;
                    case QNetworkInterface::InterfaceType::CanBus : ui->labNetwType->setText("CAN");break;
                    default : ui->labNetwType->setText("Unknown");break;
                    }


                    //todo ui->labNetwDNS
                    ui->labNetwSpeed->setText(DataVec[i].speedLinkInfo);

                }



                QVector<double> tmpRxY(DataVec[i].currentRxData.begin(),DataVec[i].currentRxData.end()-8);
                QVector<double> tmpTxY(DataVec[i].currentTxData.begin(),DataVec[i].currentTxData.end()-8);
                m_curveDataNetworkPtrVec[0]->setSamples(xAchsisBase600,tmpRxY);
                m_curveDataNetworkPtrVec[1]->setSamples(xAchsisBase600,tmpTxY);

            }

        }
        slowCnter++;


        // }

        ui->plotNetwork->replot();

    } catch (std::exception& e) {
        qDebug() << "execption when plotting network acitivity: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when plotting network acitivity";
    }
}

void LTM::update_static_network_info_from(StatTypes::NetworkData& networkData){
    // ui->labName =

}




