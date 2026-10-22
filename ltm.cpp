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
#include "processgroup.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHash>
#include <qwt_scale_engine.h>
#include "ramitemwidget.h"
#include <QProcess>
#include <QDir>
#include <cmath>
#include <cstdlib>
#include <QObject>
#include <QProcess>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


LTM::LTM(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LTM)
    , updateprocess(nullptr)

{
    std::cout << "[LTM ctor] entering" << std::endl;

    ui->setupUi(this);
    std::cout << "[LTM ctor] setupUi done" << std::endl;

    connect(ui->actionUpdate,&QAction::triggered,this, &LTM::selfUpdate);
    connect(ui->actionInfo,&QAction::triggered,this, &LTM::printInfo);

    // ProcessSortTree handles its own sorting setup in its constructor
    std::cout << "[LTM ctor] actions connected" << std::endl;


    // Setup headers once
    ui->systemdTable->setColumnCount(5);
    ui->systemdTable->setHorizontalHeaderLabels({"UNIT", "LOAD", "ACTIVE", "SUB", "DESCRIPTION"});
    ui->systemdTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(&serviceUpdater, &QTimer::timeout, this, &LTM::load_services);
    serviceUpdater.setInterval(1000);
    serviceUpdater.start();



    //setting up plots
    try {
        std::cout << "[LTM ctor] setup_cpu_plots..." << std::endl;
        setup_cpu_plots();
        std::cout << "[LTM ctor] setup_cpu_plots done" << std::endl;
        std::cout << "[LTM ctor] setup_network_plots..." << std::endl;
        setup_network_plots();
        std::cout << "[LTM ctor] setup_network_plots done" << std::endl;
        std::cout << "[LTM ctor] setup_ram_plots..." << std::endl;
        setup_ram_plots();
        std::cout << "[LTM ctor] setup_ram_plots done" << std::endl;
        std::cout << "[LTM ctor] setup_disk_plots..." << std::endl;
        setup_disk_plots();
        std::cout << "[LTM ctor] setup_disk_plots done" << std::endl;
        for(double i=0.0; i<600; i++){

            xAchsisBase600.push_back(i);
        }
        std::cout << "[LTM ctor] xAchsisBase600 filled" << std::endl;
    } catch (std::exception& e) {
        qDebug() << "execption when setting up plots: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured while setting up plots";
    }




    try {
        std::cout << "[LTM ctor] starting CPUStatReader..." << std::endl;
        m_CPUStatReaderT.start();
    } catch (std::exception& e) {
        qDebug() << "execption when initialising CPUreader: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when initialising CPUreader";
    }
       std::cout << "[LTM ctor] cpu stat reader started" << std::endl;




    try {
        std::cout << "[LTM ctor] starting RamStatReader..." << std::endl;
        m_RamStatReaderT.start();
    } catch (std::exception& e) {
        qDebug() << "execption when initialising RamStatReader: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when initialising RamStatReader";
    }
    std::cout << "[LTM ctor] ram stat reader started" << std::endl;



    try {
        std::cout << "[LTM ctor] starting DiskStatReader..." << std::endl;
        m_DiskStatReaderT.start();
    } catch (std::exception& e) {
        qDebug() << "execption when initialising DiskStatReader: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when initialising DiskStatReader";
    }
       std::cout << "[LTM ctor] disk stat reader started" << std::endl;


    try {
        std::cout << "[LTM ctor] starting NetworkStatReader..." << std::endl;
        m_NetworkStatReaderT.start();
    } catch (std::exception& e) {
        qDebug() << "execption when initialising NetworkStatReader: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when initialising NetworkStatReader";
    }
        std::cout << "[LTM ctor] network stat reader started" << std::endl;



    try {
        std::cout << "[LTM ctor] starting ProcessStatReader..." << std::endl;
        m_ProcessStatReaderT.start();
    } catch (std::exception& e) {
        qDebug() << "execption when initialising ProcessStatReader: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when initialising ProcessStatReader";
    }
        std::cout << "[LTM ctor] process stat reader started" << std::endl;


    //assembling the left ItemWidget with currently present devices
    std::cout << "[LTM ctor] assembling the left ItemWidget with currently present devices" << std::endl;
try {
    std::cout << "[LTM ctor] creating CPU entry" << std::endl;
    m_cpuItemWidgetPtr = new CPUItemWidget("CPU",ui->listWidget);
    std::cout << "[LTM ctor] creating RAM entry" << std::endl;
    m_RamItemWidgetPtr =   new RamItemWidget("RAM",ui->listWidget);
   // m_SwapItemWidgetPtr = new RamItemWidget("SWAP",ui->listWidget);
    std::cout << "[LTM ctor] creating DISK entries (count=" << m_DiskStatReaderT.getDeviceCount() << ")" << std::endl;

    for(int i=0; i<m_DiskStatReaderT.getDeviceCount(); i++){
        m_DiskItemWidgetPtrVec.push_back(new DiskItemWidget(m_DiskStatReaderT.getStatData(i).diskType,m_DiskStatReaderT.getStatData(i).diskName,ui->listWidget));
    }
    std::cout << "[LTM ctor] creating Network entries (count=" << m_NetworkStatReaderT.getDeviceCount() << ")" << std::endl;
    for(int i=0; i<m_NetworkStatReaderT.getDeviceCount(); i++){
        m_EthItemWidgetPtrVec.push_back(new NetworkItemWidget(m_NetworkStatReaderT.getStatData(i).AdapterName,ui->listWidget));
    }

    std::cout << "[LTM ctor] listWidget count=" << ui->listWidget->count() << std::endl;
    if(ui->listWidget->count() == 0){
        std::cerr << "[LTM ctor] FATAL: listWidget is empty - no items to select!" << std::endl;
    } else {
        std::cout << "[LTM ctor] selecting item(0): " << ui->listWidget->item(0)->text().toStdString() << std::endl;
        ui->listWidget->item(0)->setSelected(true);
        std::cout << "[LTM ctor] calling on_listWidget_itemClicked..." << std::endl;
        on_listWidget_itemClicked(ui->listWidget->item(0));
        std::cout << "[LTM ctor] on_listWidget_itemClicked done" << std::endl;
    }
    } catch (std::exception& e) {
        qDebug() << "execption when assembling ItemWidget: " << e.what();
    } catch (...){
        qDebug() << "unknown execption occured when assembling itemWidget";
    }



    if(connect(&m_CPUStatReaderT,&CPUStatReader::data_ready,this,&LTM::plot_cpu_activity ,Qt::ConnectionType::QueuedConnection))
        std::cout << "[LTM ctor] cpu stat reader connected" << std::endl;

    if(connect(&m_RamStatReaderT,&RamStatReader::data_ready,this,&LTM::plot_ram_activity ,Qt::ConnectionType::QueuedConnection))
        std::cout << "[LTM ctor] ram stat reader connected" << std::endl;

    if(connect(&m_DiskStatReaderT,&DiskStatReader::data_ready,this,&LTM::plot_disk_activity ,Qt::ConnectionType::QueuedConnection))
        std::cout << "[LTM ctor] disk stat reader connected" << std::endl;

    if(connect(&m_NetworkStatReaderT,&NetworkStatReader::data_ready,this,&LTM::plot_network_activity ,Qt::ConnectionType::QueuedConnection))
        std::cout << "[LTM ctor] network stat reader connected" << std::endl;

    if(connect(&m_ProcessStatReaderT,&ProcessStatReader::data_ready,this,&LTM::handle_process_data ,Qt::ConnectionType::QueuedConnection))
        std::cout << "[LTM ctor] process stat reader connected" << std::endl;

    std::cout << "[LTM ctor] DONE" << std::endl;
}

LTM::~LTM()
{
    delete ui;
}





void LTM::deselectAllCategories()
{

}





void LTM::on_listWidget_itemClicked(QListWidgetItem *item){
    std::cout << "[LTM::on_listWidget_itemClicked] called, item=" << (item ? item->text().toStdString() : "nullptr") << std::endl;
    auto perfItemWidetPtr = dynamic_cast<PerfItemWidget*>(item);
    if(!perfItemWidetPtr){
        std::cerr << "[LTM::on_listWidget_itemClicked] dynamic_cast to PerfItemWidget returned nullptr!" << std::endl;
        return;
    }

    switch(perfItemWidetPtr->getPlotType()){
    case PlotType::CPU : ui->stackedWidget->setCurrentIndex(0); break;
    case PlotType::RAM : ui->stackedWidget->setCurrentIndex(1); break;
    case PlotType::DISK : ui->stackedWidget->setCurrentIndex(2); break;
    case PlotType::Ethernet : ui->stackedWidget->setCurrentIndex(3); break;
    case PlotType::WIFI : ui->stackedWidget->setCurrentIndex(4); break;
    case PlotType::GPU : ui->stackedWidget->setCurrentIndex(5); break;
    }
    std::cout << "[LTM::on_listWidget_itemClicked] done" << std::endl;

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
    updateprocess->setProcessChannelMode(QProcess::MergedChannels);
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
    ui->updateText->appendPlainText("Version 3.1.0");
    ui->updateText->appendPlainText("Published under GPLv3, no liabilites or whatsover are taken");
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


// Update one tree item in place: keep the existing item (and its children),
// only refresh the displayed values. Returns the (possibly new) item.
static QTreeWidgetItem* set_process_item_values(QTreeWidgetItem* item,
                                               const QString& firstColumn,
                                               const QString& user,
                                               const QString& cpu,
                                               const QString& memory,
                                               const QString& disk,
                                               const QString& net,
                                               const QString& name,
                                               const QString& rawCpu,
                                               const QString& rawMemory,
                                               const QString& rawDisk,
                                               const QString& rawNet)
{
    item->setText(0, firstColumn);
    item->setText(1, user);
    item->setText(2, cpu);
    item->setText(3, memory);
    item->setText(4, disk);
    item->setText(5, net);
    item->setText(6, name);
    // raw numbers in UserRole so sortingEnabled() sorts numerically, not lexicographically
    item->setData(2, Qt::UserRole, rawCpu);
    item->setData(3, Qt::UserRole, rawMemory);
    item->setData(4, Qt::UserRole, rawDisk);
    item->setData(5, Qt::UserRole, rawNet);
    return item;
}

void LTM::handle_process_data()
{
    try {
        auto& dataVec = m_ProcessStatReaderT.m_DataVec;

        QVector<StatTypes::ProcessGroup> groups;
        QHash<int, const StatTypes::ProcessData*> pidLookup;
        {
            std::lock_guard<std::mutex> lck(m_ProcessStatReaderT.m_DataVecMutex);
            groups = aggregate_processes(dataVec);
            for (const auto& rec : dataVec) pidLookup.insert(rec.pid, &rec);
        }

        ui->treeWProcess->setUpdatesEnabled(false);

        // Build a set of active group names for removing stale items
        QSet<QString> activeGroupNames;
        for (const auto& g : groups) activeGroupNames.insert(g.name);

        // Remove top-level items whose group no longer exists
        for (int r = ui->treeWProcess->topLevelItemCount() - 1; r >= 0; r--) {
            QTreeWidgetItem* item = ui->treeWProcess->topLevelItem(r);
            if (!activeGroupNames.contains(item->data(0, Qt::UserRole).toString()))
                delete item;
        }

        // Update or create each group in-place (preserves expansion state)
        for (const auto& group : groups) {
            QTreeWidgetItem* item = nullptr;
            for (int r = 0; r < ui->treeWProcess->topLevelItemCount(); r++) {
                if (ui->treeWProcess->topLevelItem(r)->data(0, Qt::UserRole).toString() == group.name) {
                    item = ui->treeWProcess->topLevelItem(r);
                    break;
                }
            }

            if (!item) {
                item = new QTreeWidgetItem();
                item->setData(0, Qt::UserRole, group.name);
                ui->treeWProcess->addTopLevelItem(item);
            }

            // Refresh group row values
            set_process_item_values(item,
                QString::number(group.pids.size()),
                group.user,
                QString::number(group.totalCpuPct*100, 'f', 1) + "%",
                QString::number(group.totalMemKiB/1024.0, 'f', 1) + " MiB",
                QString::number(group.totalDiskKiBs/1024.0, 'f', 1) + " MiB/s",
                m_ProcessStatReaderT.noRootPriv ? QString("need root")
                                                : QString::number(group.totalNetKiBs/1024.0, 'f', 1) + " MiB/s",
                group.displayName,
                QString::number(group.totalCpuPct*100),
                QString::number(group.totalMemKiB/1024.0),
                QString::number(group.totalDiskKiBs/1024.0),
                QString::number(group.totalNetKiBs/1024.0));

            // --- children: remove stale, update existing, add new ---
            QSet<int> activePids;
            for (int pid : group.pids) activePids.insert(pid);

            for (int c = item->childCount() - 1; c >= 0; c--) {
                int pid = item->child(c)->data(0, Qt::UserRole).toInt();
                if (!activePids.contains(pid))
                    delete item->child(c);
            }

            for (int pid : group.pids) {
                auto it = pidLookup.find(pid);
                if (it == pidLookup.end()) continue;
                const auto& proc = *it.value();

                QTreeWidgetItem* child = nullptr;
                for (int c = 0; c < item->childCount(); c++) {
                    if (item->child(c)->data(0, Qt::UserRole).toInt() == pid) {
                        child = item->child(c);
                        break;
                    }
                }
                if (!child) {
                    child = new QTreeWidgetItem();
                    child->setData(0, Qt::UserRole, pid);
                    item->addChild(child);
                }

                set_process_item_values(child,
                    QString::number(pid),
                    proc.user,
                    QString::number(proc.currentCPUPct*100, 'f', 1) + "%",
                    QString::number(proc.currentMemKiB/1024.0, 'f', 1) + " MiB",
                    QString::number((proc.currentReadKiBsDiskData+proc.currentWriteKiBsDiskData)/1024.0, 'f', 1) + " MiB/s",
                    m_ProcessStatReaderT.noRootPriv ? QString("need root")
                                                    : QString::number((proc.currentReadKiBsNetData+proc.currentWriteKiBsNetData)/1024.0, 'f', 1) + " MiB/s",
                    proc.name,
                    QString::number(proc.currentCPUPct*100),
                    QString::number(proc.currentMemKiB/1024.0),
                    QString::number((proc.currentReadKiBsDiskData+proc.currentWriteKiBsDiskData)/1024.0),
                    QString::number((proc.currentReadKiBsNetData+proc.currentWriteKiBsNetData)/1024.0));
            }
        }

        // Apply numeric sort after updating values
        ui->treeWProcess->applyNumericSort();

        ui->treeWProcess->setUpdatesEnabled(true);

    } catch (std::exception& e) {
        qDebug() << "exception when assembling process data: " << e.what();
    } catch (...) {
        qDebug() << "unknown exception occured when assembling process data";
    }
}

void LTM::on_butExpandAllProcesses_clicked()
{
    for(int r=0; r<ui->treeWProcess->topLevelItemCount(); r++)
        ui->treeWProcess->topLevelItem(r)->setExpanded(true);
}

void LTM::on_butCollapseAllProcesses_clicked()
{
    for(int r=0; r<ui->treeWProcess->topLevelItemCount(); r++)
        ui->treeWProcess->topLevelItem(r)->setExpanded(false);
}


void LTM::on_butKillProcess_clicked()
{
    std::lock_guard<std::mutex> lck(m_ProcessStatReaderT.m_DataVecMutex); //just to be sure not to get current selcted item while table is being rebuilt

    QTreeWidgetItem* sel = ui->treeWProcess->currentItem();
    if(sel == nullptr) return;

    // collect the pids belonging to the selection: a group (top level) or a single pid (child)
    QVector<int> pids;
    if(sel->parent() == nullptr){
        for(int c=0; c<sel->childCount(); c++)
            pids.push_back(sel->child(c)->data(0, Qt::UserRole).toInt());
    } else {
        pids.push_back(sel->data(0, Qt::UserRole).toInt());
    }
    if(pids.isEmpty()) return;

    for(int pid : pids)
        system((std::string("kill -9 ") + QString::number(pid).toStdString()).c_str());
}

void LTM::load_services() {
    QProcess process;
    process.start("systemctl", {"list-units", "--type=service", "--all", "--no-pager", "--output=json"});
    if (!process.waitForFinished(5000)) {
        // handle error
        return;
    }

    QByteArray jsonOutput = process.readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(jsonOutput);

    if (!doc.isArray()) return;

    QJsonArray array = doc.array();
    ui->systemdTable->clearContents();
    ui->systemdTable->setRowCount(array.size());

    int row = 0;
    for (const QJsonValue& val : array) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        QString unit = obj["unit"].toString();
        QString load = obj["load"].toString();
        QString active = obj["active"].toString();
        QString sub = obj["sub"].toString();
        QString description = obj["description"].toString();

        ui->systemdTable->setItem(row, 0, new QTableWidgetItem(unit));
        ui->systemdTable->setItem(row, 1, new QTableWidgetItem(load));
        ui->systemdTable->setItem(row, 2, new QTableWidgetItem(active));
        ui->systemdTable->setItem(row, 3, new QTableWidgetItem(sub));
        ui->systemdTable->setItem(row, 4, new QTableWidgetItem(description));
        row++;
    }

    ui->systemdTable->setRowCount(row); // In case some rows were skipped
}
