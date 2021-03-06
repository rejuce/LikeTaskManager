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
    // ui->plotRam->setAxisAutoScale ( QwtPlot::yRight, true );
    // ui->plotRam->axisScaleEngine(QwtPlot::yRight)->setAttribute(QwtScaleEngine::Attribute::Symmetric, false);
    //ui->plotRam->axisScaleEngine(QwtPlot::yRight)->setMargins(1,1);
    ui->plotRam->setAxisAutoScale ( QwtPlot::yRight, false );
    ui->plotRam->setAxisScale ( QwtPlot::yRight, 0,100 );
    ui->plotRam->setAxisTitle(QwtPlot::yRight,"%-Used");

    m_gridRamPtr->setPen(QColor::fromRgb(0xEF,0xF0,0xF1));
    m_gridRamPtr->enableYMin(false);
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

    m_curveDataRamPtrVec[0]->setPen(Qt::magenta, 3 );
    QBrush col;
    col.setColor(Qt::darkMagenta);
    col.setStyle(Qt::SolidPattern);
    m_curveDataRamPtrVec[0]->setBrush(col);
    m_curveDataRamPtrVec[1]->setPen(Qt::darkGreen, 3);

    ui->labMemHeader->setText(QString::number(m_RamStatReaderT.m_DataVec[0].RamSizeKiB*9.53674e-7,'f',1)+ " GiB");

    ui->plotRam->replot();
}

void LTM::plot_ram_activity()
{
    static int slowCnter =0;

    try{

        auto& DataVec = m_RamStatReaderT.m_DataVec;

        // for(size_t i=0; i<DataVec.size(); i++){

        for(int i=0; i<DataVec.length(); i++){

            std::lock_guard<std::mutex> lock(m_RamStatReaderT.m_DataVecMutex);


            if(slowCnter%NetworkStatReader::m_widgetDataModulus==0) {
                double usedGib = DataVec[i].currentUsage.back()*DataVec[i].RamSizeKiB*9.53674e-7/100;
                double totalGib= DataVec[i].RamSizeKiB*9.53674e-7;
                m_RamItemWidgetPtr->update_data(usedGib, totalGib);

                ui->labMemory->setText(QString::number(usedGib,'f',1) + " GiB (" +  QString::number(usedGib/totalGib*100,'f',1) + "%) of " + QString::number(totalGib,'f',1) + " GiB");

                double usedSwapGib = DataVec[i].currentSwap.back()*DataVec[i].SwapSizeKiB*9.53674e-7/100.0;
                double totalSwapGib= DataVec[i].SwapSizeKiB*9.53674e-7;
                ui->labSwap->setText(QString::number(usedSwapGib,'f',1) + " GiB (" +  QString::number(usedSwapGib/totalSwapGib*100,'f',1) + "%) of " + QString::number(totalSwapGib,'f',1) + " GiB");

            }


            QVector<double> tmpRam(DataVec[i].currentUsage.begin(),DataVec[i].currentUsage.end()-8);
            QVector<double> tmpSwap(DataVec[i].currentSwap.begin(),DataVec[i].currentSwap.end()-8);
            m_curveDataRamPtrVec[0]->setSamples(xAchsisBase600,tmpRam);
            m_curveDataRamPtrVec[1]->setSamples(xAchsisBase600,tmpSwap);

        }


        slowCnter++;


        // }

        ui->plotRam->replot();

    } catch (std::exception& e) {
        qDebug() << "execption when plotting ram acitivity: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when plotting ram acitivity";
    }

}
