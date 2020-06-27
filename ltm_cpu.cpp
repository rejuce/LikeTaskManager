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
    m_gridCpuPtr->setYAxis(QwtPlot::yRight);
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
        singleCPUCurve->setPen(colovec[colorindex], 1.5 );  //1.5 bec it looks best...
        // singleCPUCurve->setStyle( QwtPlotCurve::CurveStyle::Dots);
        singleCPUCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
        singleCPUCurve->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
        colorindex++;
    }
    m_curveDataCpuPtrVec[0]->setPen(Qt::white, 4);//cpu sum

    ui->plotCpu->replot();


    ui->labCPUHeader->setText(m_CPUStatReaderT.model);
    ui->labCPUPhysCores->setText(m_CPUStatReaderT.CPUCoresPhys);
    ui->labCPULogCores->setText(m_CPUStatReaderT.CPUCoresLogical);
    ui->labCPUCache->setText(m_CPUStatReaderT.cacheSize);



}







void LTM::plot_cpu_activity(){

    try {


        auto& DataVec = m_CPUStatReaderT.m_DataVec;

        for(size_t i=0; i<DataVec.size(); i++){

            std::lock_guard<std::mutex> lock(m_CPUStatReaderT.m_DataVecMutex);
            if(i>0){

                m_curveDataCpuPtrVec[i]->setZ(0);
            }

            else{
                static int slowCnter = 0;
                if(slowCnter%DiskStatReader::m_widgetDataModulus==0){
                    //8 items from the back where it is already smoothed
                    m_cpuItemWidgetPtr->update_data(DataVec[i].currentActivityData[DataVec[i].currentActivityData.size()-1],*(DataVec[i].currentClockSpeed.end()-8)/1000);
                    if(i==0){
                        ui->labCPULoad->setText(QString::number(*(DataVec[i].currentActivityData.end()-8),'f',2)+"%");
                        ui->labCPUClock->setText(QString::number(*(DataVec[i].currentClockSpeed.end()-8)/1000,'f',2)+" GHz");
                    }

                }

                m_curveDataCpuPtrVec[0]->setZ(2);
                slowCnter++;
            }

            QVector<double> tmpY(DataVec[i].currentActivityData.begin(),DataVec[i].currentActivityData.end());
            m_curveDataCpuPtrVec[i]->setSamples(xAchsisBase600,tmpY);

        }

        ui->plotCpu->replot();

    } catch (std::exception& e) {
        qDebug() << "execption when plotting cpu acitivity: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when plotting cpu acitivity";
    }

}




