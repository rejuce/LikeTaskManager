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
#include "processstatreader.h"
#include <qwt_scale_engine.h>
#include "ramitemwidget.h"
#include <QProcess>
#include <QDir>
#include <cmath>


LTM::LTM(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LTM)

{
    ui->setupUi(this);

    connect(ui->actionUpdate,&QAction::triggered,this, &LTM::selfUpdate);
    connect(ui->actionInfo,&QAction::triggered,this, &LTM::printInfo);

   // for(size_t i=0; i<cputCnt+1; i++){
   // m_curveDataCpuPtrVec[i]->setCurveFitter(m_curveFitterPtrVec[i].get());
      //  m_curveDataCpuPtrVec[i]->setCurveAttribute(QwtPlotCurve::CurveAttribute::Fitted,true);
  //  }

    setup_cpu_plots();
    setup_network_plots();
    setup_ram_plots();
    setup_disk_plots();

    for(double i=0.0; i<600; i++){
        xAchsisBase600.push_back(i);

    }




    if(connect(&m_CPUStatReaderT,&CPUStatReader::data_ready,this,&LTM::plot_cpu_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "cpu stat reader started";

    m_CPUStatReaderT.start();


    if(connect(&m_RamStatReaderT,&RamStatReader::data_ready,this,&LTM::plot_ram_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "ram stat reader started";

    m_RamStatReaderT.start();


    if(connect(&m_DiskStatReaderT,&DiskStatReader::data_ready,this,&LTM::plot_disk_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "disk stat reader started";

    m_DiskStatReaderT.start();


    if(connect(&m_NetworkStatReaderT,&NetworkStatReader::data_ready,this,&LTM::plot_network_activity ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "network stat reader started";

    m_NetworkStatReaderT.start();


    if(connect(&m_ProcessStatReaderT,&ProcessStatReader::data_ready,this,&LTM::handle_process_data ,Qt::ConnectionType::QueuedConnection))
        qDebug()<< "network stat reader started";

    m_ProcessStatReaderT.start();


//testEntry->set_cpu_clock(20);
 //   ui->listWidget->addItem(rowW);
    m_cpuItemWidgetPtr = new CPUItemWidget("CPU",ui->listWidget);





     m_RamItemWidgetPtr =   new RamItemWidget("RAM",ui->listWidget);


     for(int i=0; i<m_DiskStatReaderT.getDeviceCount(); i++){
         m_DiskItemWidgetPtrVec.push_back(new DiskItemWidget(m_DiskStatReaderT.getStatData(i).diskType,m_DiskStatReaderT.getStatData(i).diskName,ui->listWidget));

     }

for(int i=0; i<m_NetworkStatReaderT.getDeviceCount(); i++){
    m_EthItemWidgetPtrVec.push_back(new NetworkItemWidget(m_NetworkStatReaderT.getStatData(i).AdapterName,ui->listWidget));

}

//ui->listWidget->setItemWidget( ui->listWidget->item(0), &m_cpuListWidget.widgetO);
ui->listWidget->item(0)->setSelected(true);
on_listWidget_itemClicked(ui->listWidget->item(0));
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
       //uto test =  ui->listWidget->currentRow();
  // dynamic_cast<CPUListViewItem*>(ui->listWidget->currentItem())->set_cpu_clock(99);
     //ui->listWidget->item(2)->setSelected(true);
//test++;
}

LTM::~LTM()
{
    delete ui;
}



void LTM::deselectAllCategories()
{

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


void LTM::selfUpdate()
{

    ui->stackedWidget->setCurrentIndex(4);
    ui->tabWidget->setCurrentIndex(0);


    updateprocess = new QProcess(this);

    //ui->updateText->show();

    qDebug() << "App path : " << qApp->applicationDirPath();
//QString binDirectory = QFileInfo(qApp->applicationDirPath()).dir().absolutePath();

    if(! QFile(qApp->applicationDirPath()+"/appimageupdatetool-x86_64.AppImage").exists()) ui->updateText->appendPlainText("ERROR: FAILED appimageupdate not packaged");
   // else if(! QFile("/usr/appimageupdatetool-x86_64.AppImage").exists()) ui->updateText->appendPlainText("ERROR: FAILED appimageupdate tool not isntalled");

    std::cout <<  "$APPIMAGE:" << qgetenv("APPIMAGE").toStdString() << std::endl;
    updateprocess->setReadChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    // updateprocess->start("./usr/AppImageUpdate.AppImage");
    ui->updateText->appendPlainText("starting updater...");
    qApp->processEvents();
    updateprocess->start(qApp->applicationDirPath()+"/appimageupdatetool-x86_64.AppImage",QStringList() <<qgetenv("APPIMAGE"));
    //updateprocess->start("/usr/appimageupdatetool-x86_64.AppImage",QStringList() <<qgetenv("APPIMAGE"));
    updateprocess->waitForStarted(1000);

    while(updateprocess->state() == QProcess::ProcessState::Running){
        if( updateprocess->waitForReadyRead(2000)){
            QString appImageUpdatePath = updateprocess->readAllStandardOutput();
            appImageUpdatePath.replace("\r","");
            ui->updateText->appendPlainText(appImageUpdatePath);
            qApp->processEvents();
        }

    }


    updateprocess->waitForFinished(500);
    QString appImageUpdatePath = updateprocess->readAllStandardOutput();
    // QString appImageErr= updateprocess->readAllStandardError();
    qDebug()<< appImageUpdatePath;// << appImageErr;

   // ui->okButton->show();



    // while(1)
    //auto state= updateprocess->state();



}

void LTM::printInfo()
{
            ui->updateText->clear();
     ui->stackedWidget->setCurrentIndex(4);
     ui->tabWidget->setCurrentIndex(0);

     ui->updateText->appendPlainText("Authored by Juergen Kratochwill 2020");
     ui->updateText->appendPlainText("Published under GPLv3");
     ui->updateText->appendPlainText("All Linux and QT related Licesing info can be accesed under: ");
     QDir license(qApp->applicationDirPath());
     license.cdUp();
     license.cd("share");
     license.cd("doc");
     ui->updateText->appendPlainText(license.path());
     ui->updateText->appendPlainText("    ");
      ui->updateText->appendPlainText("Additonal Attributions:");
      ui->updateText->appendPlainText("QDarkStyleSheet Project by Colin Duquesnoy: https://github.com/ColinDuquesnoy/QDarkStyleSheet");
      ui->updateText->appendPlainText("Icons from: https://www.flaticon.com/Freepik");

       ui->updateText->appendPlainText("Net I/O pre process nethgos/libnethog from https://github.com/raboof/nethogs/ ");


}

void LTM::on_butUpdateOK_clicked()
{


    on_listWidget_itemClicked(ui->listWidget->currentItem());
}



void LTM::handle_process_data()
{
    auto& dataVec = m_ProcessStatReaderT.m_DataVec;

    QItemSelectionModel *select = ui->tableWProcess->selectionModel();
    //int curSelectedRow = select->selectedRows().first().row();

    int currentSelectedPID = 0;

    if(select->selectedRows().size()>0){
        int curSelectedRow= select->selectedRows().first().row();
        currentSelectedPID = ui->tableWProcess->item(curSelectedRow,0)->data(Qt::DisplayRole).toInt();
    }
    else  {
        currentSelectedPID =-1;
    }






    ui->tableWProcess->setRowCount(dataVec.size());

     std::lock_guard<std::mutex> lck(m_ProcessStatReaderT.m_DataVecMutex);
   ui->tableWProcess->setSortingEnabled(false);
   //ui->tableWProcess->clear();

     for(int pr=0; pr<dataVec.size();pr++){
        if(ui->tableWProcess->item(pr,0)==nullptr) {//does not exist yet -> create whole line
            for(int k=0; k<7; k++){
                QTableWidgetItem *newItem = new QTableWidgetItem();
                   ui->tableWProcess->setItem(pr, k, newItem);
                   ui->tableWProcess->item(pr,k)->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            }


        }


        ui->tableWProcess->item(pr,0)->setData(Qt::DisplayRole,dataVec[pr].pid);
         ui->tableWProcess->setColumnWidth(0,60);

        ui->tableWProcess->item(pr,1)->setData(Qt::DisplayRole,dataVec[pr].user);
        ui->tableWProcess->setColumnWidth(1,80);

        ui->tableWProcess->item(pr,2)->setData(Qt::DisplayRole,dataVec[pr].currentCPUPct );
      //  ui->tableWProcess->item(pr,2)->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui->tableWProcess->setColumnWidth(2,70);

    //    ui->tableWProcess->item(pr,3)->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui->tableWProcess->item(pr,3)->setData(Qt::DisplayRole,dataVec[pr].currentMemKiB/1024);
        ui->tableWProcess->setColumnWidth(3,100);



        ui->tableWProcess->item(pr,4)->setData(Qt::DisplayRole,(dataVec[pr].currentReadKiBsDiskData+dataVec[pr].currentWriteKiBsDiskData)/1024);
        ui->tableWProcess->setColumnWidth(4,100);
        ui->tableWProcess->item(pr,5)->setData(Qt::DisplayRole,(dataVec[pr].currentReadKiBsNetData+dataVec[pr].currentWriteKiBsNetData)/1024);
        ui->tableWProcess->setColumnWidth(5,100);

        ui->tableWProcess->item(pr,6)->setData(Qt::DisplayRole,dataVec[pr].name);
        ui->tableWProcess->setColumnWidth(6,150);




    }


        ui->tableWProcess->setSortingEnabled(true); //quickly sort it

       ui->tableWProcess->setSortingEnabled(false); //disable sorting again and adjust cell formants
        ui->tableWProcess->horizontalHeader()->setSortIndicatorShown(true);


       for(int pr=0; pr<dataVec.size();pr++){
           //fix selection
           if(ui->tableWProcess->item(pr,0)->data(Qt::DisplayRole).toInt()==currentSelectedPID){
               //select row again
               ui->tableWProcess->selectRow(pr);
               //select->row
           }

           ui->tableWProcess->item(pr,3)->setData(Qt::DisplayRole,QString::number(ui->tableWProcess->item(pr,3)->data(Qt::DisplayRole).toFloat(),'f',1) + " MiB");
           ui->tableWProcess->item(pr,2)->setData(Qt::DisplayRole,QString::number(ui->tableWProcess->item(pr,2)->data(Qt::DisplayRole).toFloat()*100,'f',1) + QString("%"));
           ui->tableWProcess->item(pr,4)->setData(Qt::DisplayRole,QString::number(ui->tableWProcess->item(pr,4)->data(Qt::DisplayRole).toFloat(),'f',1) + QString(" MiB/s"));

           if(!m_ProcessStatReaderT.noRootPriv)
           ui->tableWProcess->item(pr,5)->setData(Qt::DisplayRole,QString::number(ui->tableWProcess->item(pr,5)->data(Qt::DisplayRole).toFloat(),'f',1) + QString(" MiB/s"));
            else ui->tableWProcess->item(pr,5)->setText("need root");

       }



    int hold=0;

}
