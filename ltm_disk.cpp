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


    ui->plotDisk->setAxisAutoScale ( QwtPlot::yLeft, false );
    ui->plotDisk->setAxisScale ( QwtPlot::yLeft, 0,100 );
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
    try{

        auto& DataVec = m_DiskStatReaderT.m_DataVec;

        // for(size_t i=0; i<DataVec.size(); i++){
        static int slowCnter = 0;
        for(int i=0; i<DataVec.size(); i++){

            std::lock_guard<std::mutex> lock(m_DiskStatReaderT.m_DataVecMutex);
            static QString currentWriteStr;
            static QString currentReadStr;
            static QString activeStr;

            if(slowCnter%DiskStatReader::m_widgetDataModulus==0) {
                //     m_DiskItemWidgetPtrVec[i]->update_data(DataVec[i].currentTxData.back(), DataVec[i].currentRxData.back());
                static const QString KBs = " KiB/s";
                static const QString MBs = " MiB/s";
                constexpr int smoothBackMargin = 8;
                double currentWrite = *(DataVec[i].currentWriteKiBsData.end()-smoothBackMargin);
                currentWriteStr = (currentWrite<1000) ? QString::number(currentWrite,'f',0) + KBs : QString::number(currentWrite/1024.0,'f',0) + MBs;
                double currentRead = (*(DataVec[i].currentReadKiBsData.end()-smoothBackMargin));
                currentReadStr = (currentRead<1000) ? QString::number(currentRead,'f',0) + KBs : QString::number(currentRead/1024.0,'f',0) + MBs;
                double active = *(DataVec[i].currentActivityData.end()-smoothBackMargin);
                activeStr = QString::number(active,'f',0);


                m_DiskItemWidgetPtrVec[i]->update_data(currentReadStr,currentWriteStr,activeStr );


            }




            //        //only if current selected corresnposds to current data index plot and update register
            if(current_disk_index_is_slected(i)){

                if(slowCnter%DiskStatReader::m_widgetDataModulus==0) {
                    ui->labDiskName->setText(DataVec[i].manufacturer);
                    ui->labDiskRead->setText(currentReadStr) ;
                    ui->labDiskWrite->setText(currentWriteStr) ;
                    ui->labActive->setText(activeStr+"%");
                    ui->labRespT->setText(QString::number(DataVec[i].averageRWRespTime,'f',1)+" ms");
                    ui->labIOPS->setText(QString::number(DataVec[i].currentIOPS,'f',0));
                    ui->labDiskCapacity->setText(QString::number(DataVec[i].sizeByte/1024/1024/1024)+" GiB");
                }



                double maxR = *std::max_element(DataVec[i].currentReadKiBsData.begin(), DataVec[i].currentReadKiBsData.end());
                double maxW = *std::max_element(DataVec[i].currentWriteKiBsData.begin(), DataVec[i].currentWriteKiBsData.end());
                double scaleFactor = 1; //output in KiB

                if(maxR<1000 && maxW < 1000){
                    scaleFactor=1;
                    ui->plotDisk->setAxisTitle(QwtPlot::yRight,"KiB/s");
                }
                else if(maxR>1000 || maxW > 1000){
                    scaleFactor=1024;
                    ui->plotDisk->setAxisTitle(QwtPlot::yRight,"MiB/s");
                }




                QVector<double> tmpRxY(DataVec[i].currentReadKiBsData.begin(),DataVec[i].currentReadKiBsData.end()-8);
                QVector<double> tmpTxY(DataVec[i].currentWriteKiBsData.begin(),DataVec[i].currentWriteKiBsData.end()-8);

                for(auto& dp : tmpRxY)
                    dp=-dp/scaleFactor;

                for(auto& dp : tmpTxY)
                    dp=dp/scaleFactor;

                QVector<double> tmpActive(DataVec[i].currentActivityData.begin(),DataVec[i].currentActivityData.end()-8);



                m_curveDataDiskPtrVec[0]->setSamples(xAchsisBase600,tmpRxY);
                m_curveDataDiskPtrVec[1]->setSamples(xAchsisBase600,tmpTxY);
                m_curveDataDiskPtrVec[2]->setSamples(xAchsisBase600,tmpActive);

            }

        }
        slowCnter++;


        // }

        ui->plotDisk->replot();

    } catch (std::exception e) {
        qDebug() << "execption when plotting disk acitivity: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when plotting disk acitivity";
    }
}



//void LTM::update_static_disk_info_from(StatTypes::DiskData& diskData){
//    // ui->labName =

//}




