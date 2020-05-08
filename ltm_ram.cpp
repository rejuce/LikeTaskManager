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


void LTM::setup_ram_plots()
{
    m_gridRamPtr = std::make_unique<QwtPlotGrid>();

    for(size_t i=0; i<2; i++){ //one curve for ram one for swap
        m_curveDataRamPtrVec.emplace_back(std::make_unique<QwtPlotCurve>());
        //   m_curveFitterPtrVec.emplace_back(std::make_unique<QwtSplineCurveFitter>());

    }
    // ui->plotCpu->setTitle("");
    // ui->plotCpu->setCanvasBackground( Qt::white );
    ui->plotRam->enableAxis( QwtPlot::yRight,true );
    ui->plotRam->enableAxis( QwtPlot::yLeft,false );
    ui->plotRam->enableAxis( QwtPlot::xBottom,false );
    ui->plotRam->setAxisAutoScale ( QwtPlot::yRight, true );
   // ui->plotRam->axisScaleEngine(QwtPlot::yRight)->setAttribute(QwtScaleEngine::Attribute::Symmetric, false);
    //ui->plotRam->axisScaleEngine(QwtPlot::yRight)->setMargins(1,1);
    ui->plotCpu->setAxisAutoScale ( QwtPlot::yRight, false );
    ui->plotCpu->setAxisScale ( QwtPlot::yRight, 0,100 );
    ui->plotRam->setAxisTitle(QwtPlot::yRight,"%-Used");

    m_gridRamPtr->setPen(QColor::fromRgb(0xEF,0xF0,0xF1));
    m_gridRamPtr->enableYMin(true);
    m_gridRamPtr->setYAxis(QwtPlot::yRight);
    m_gridRamPtr->attach(ui->plotRam);


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
    for(auto& singleRamDataCurve : m_curveDataRamPtrVec){
        singleRamDataCurve->attach(ui->plotRam);
        // singleCPUCurve->setStyle( QwtPlotCurve::CurveStyle::Dots);
        singleRamDataCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
        singleRamDataCurve->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
    }

    m_curveDataRamPtrVec[0]->setPen(Qt::darkMagenta, 3 );
    m_curveDataRamPtrVec[1]->setPen(Qt::darkGreen, 3);



    ui->plotRam->replot();
}

void LTM::plot_ram_activity()
{

    auto& DataVec = m_RamStatReaderT.m_DataVec;

    // for(size_t i=0; i<DataVec.size(); i++){
    static int slowCnter = 0;
    for(int i=0; i<DataVec.size(); i++){

        std::lock_guard<std::mutex> lock(m_RamStatReaderT.m_DataVecMutex);


       // if(slowCnter%m_NetworkStatReaderT.m_widgetDataModulus==0) {
      //      m_EthItemWidgetPtrVec[i]->update_data(DataVec[i].currentTxData.back(), DataVec[i].currentRxData.back());
     //   }


            QVector<double> tmpRam(DataVec[i].currentUsage.begin(),DataVec[i].currentUsage.end()-8);
            QVector<double> tmpSwap(DataVec[i].currentSwap.begin(),DataVec[i].currentSwap.end()-8);
            m_curveDataNetworkPtrVec[0]->setSamples(xAchsisBase600,tmpRam);
            m_curveDataNetworkPtrVec[1]->setSamples(xAchsisBase600,tmpSwap);

        }


  //  slowCnter++;


    // }

    ui->plotRam->replot();

}
