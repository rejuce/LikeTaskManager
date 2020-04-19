#include "ltm.h"
#include "ui_ltm.h"
#include <memory>
#include <iostream>
#include "cpustats.h"
#include <QThread>
#include <qwt_curve_fitter.h>

#include "cpuitemwidget.h"


LTM::LTM(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LTM)

{
    ui->setupUi(this);
    CPUListItem = new CPUItemWidget("CPU",ui->listWidget);
    ui->listWidget->item(0)->setSelected(true);


    m_gridCpuPtr = std::make_unique<QwtPlotGrid>();
    size_t cputCnt = CPUStats::get_cpu_count();

    for(size_t i=0; i<cputCnt+1; i++){//+ for sum of all
        m_curveDataCpuPtrVec.emplace_back(std::make_unique<QwtPlotCurve>());
        //   m_curveFitterPtrVec.emplace_back(std::make_unique<QwtSplineCurveFitter>());

    }

    for(size_t i=0; i<cputCnt+1; i++){
        // m_curveDataCpuPtrVec[i]->setCurveFitter(m_curveFitterPtrVec[i].get());
        //  m_curveDataCpuPtrVec[i]->setCurveAttribute(QwtPlotCurve::CurveAttribute::Fitted,true);
    }

    setup_cpu_plots();

    for(double i=0.0; i<600; i++){
        xAchsisBase600.push_back(i);

    }

    CPUStats::get_cpus_activity();//dummy read to populate


    //testEntry->set_cpu_clock(20);
    //   ui->listWidget->addItem(rowW);



    auto ptr = new CPUItemWidget("CPUItem0",ui->listWidget);
    //ptr->set_cpu_activity(1000);

    auto ptr2 = new CPUItemWidget("CPUItem1",ui->listWidget);
    //ptr->set_cpu_activity(100);


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

void LTM::deselectAllCategories()
{

}


void LTM::on_pushButton_clicked()
{
    while(1){
        qApp->processEvents();
        auto currentCPU = CPUStats::get_cpus_activity();
        static std::vector<QVector<double>> cpuYPlotData(currentCPU.size(),QVector<double>(xAchsisBase600.size(),0));
        for(size_t i=0; i<currentCPU.size(); i++){
            cpuYPlotData[i].pop_front();
            cpuYPlotData[i].push_back(currentCPU[i].second);

            //filtering first order
            //for(int k = +1; k < cpuYPlotData[i].size()-1; k++)
            //int k = cpuYPlotData[i].size()-2;
            //     cpuYPlotData[i][k] = (cpuYPlotData[i][k-1]+ cpuYPlotData[i][k] +cpuYPlotData[i][k+1] )/ 3.0;

            //filtering 2nd order to power n
            int k = cpuYPlotData[i].size()-3;
            for(int n=8; n>=0; n--){
                //for(int k = +2; k < cpuYPlotData[i].size()-2; k++)
                cpuYPlotData[i][k] = (cpuYPlotData[i][k-2] + cpuYPlotData[i][k-1] + cpuYPlotData[i][k] +cpuYPlotData[i][k+1] +cpuYPlotData[i][k+2] )/ 5.0;
                k--;
            }




            if(i>0){

                m_curveDataCpuPtrVec[i]->setZ(0);
            }

            else{
                static int slowCnter = 0;
                if(slowCnter%5==0) CPUListItem->update_data(cpuYPlotData[i][cpuYPlotData[i].size()-3], 4.05);
                //auto val = cpuYPlotData[i][cpuYPlotData[i].size()-1];

                // m_cpuListWidget.m_clockSpeed->setNum(val);
                m_curveDataCpuPtrVec[0]->setZ(2);
                slowCnter++;
            }

            m_curveDataCpuPtrVec[i]->setSamples(xAchsisBase600,cpuYPlotData[i]);

        }
        QThread::msleep(100);
        ui->plotCpu->replot();

    }

}

