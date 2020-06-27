#include "diskstatreader.h"
#include <QDir>
#include <mutex>
#include <QTextStream>
#include <QThread>
#include <QElapsedTimer>

DiskStatReader::DiskStatReader(QObject *parent) : QObject(parent), StatReader("DiskStatReader")
{
    update_disk_list();

}


void DiskStatReader::measure_main_loop()
{
QElapsedTimer cycleTimer;

    while(!m_quit){
        cycleTimer.restart();

        QFile f("/proc/diskstats");
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        if(f.isOpen()){
            QTextStream in(&f);

            QString line = in.readLine().simplified();
            while(!line.isNull()){
                auto stringlist = line.split(" ");

                auto* dataptr = find_disk_name_in_data(stringlist[2]);

                if(dataptr!=nullptr){
                    dataptr->currentStats.readsComplete = stringlist[3].toInt();
                    dataptr->currentStats.readsMerged = stringlist[4].toInt();
                    dataptr->currentStats.secRead = stringlist[5].toInt();
                    dataptr->currentStats.readTimeMs = stringlist[6].toInt();

                    dataptr->currentStats.writeComplete = stringlist[7].toInt();
                    dataptr->currentStats.writeMerged = stringlist[8].toInt();
                    dataptr->currentStats.secWrite = stringlist[9].toInt();
                    dataptr->currentStats.writeTimeMs = stringlist[10].toInt();

                    dataptr->currentStats.IOTime = stringlist[12].toInt();

                    if(dataptr->firstIteration) {dataptr->previousStats = dataptr->currentStats; dataptr->firstIteration=false;}

                    std::lock_guard<std::mutex> lck(m_DataVecMutex);
                    dataptr->currentReadKiBsData.pop_front();
                    //byte blocksize 512 and unit conversion
                    dataptr->currentReadKiBsData.push_back(512*(dataptr->currentStats.secRead-dataptr->previousStats.secRead)/m_cycleTimeMs/1024.0*1000);

                    dataptr->currentWriteKiBsData.pop_front();
                    dataptr->currentWriteKiBsData.push_back(512*(dataptr->currentStats.secWrite-dataptr->previousStats.secWrite)/m_cycleTimeMs/1024.0*1000);

                    dataptr->currentActivityData.pop_front();
                    dataptr->currentActivityData.push_back(100*(dataptr->currentStats.IOTime-dataptr->previousStats.IOTime)/(m_cycleTimeMs)); //corremtopm factpr

                    filter_data_deque(dataptr->currentReadKiBsData);
                    filter_data_deque( dataptr->currentWriteKiBsData);
                    filter_data_deque( dataptr->currentActivityData);


                    int dReadComplete =  (dataptr->currentStats.readsComplete - dataptr->previousStats.readsComplete);
                    int dWriteComplete =  (dataptr->currentStats.writeComplete - dataptr->previousStats.writeComplete);

                    dataptr->currentIOPS = ((  dReadComplete +dWriteComplete)/m_cycleTimeMs)*1000;

                    int dReadMerged =  (dataptr->currentStats.readsMerged - dataptr->previousStats.readsMerged);
                    int dWriteMerged =  (dataptr->currentStats.writeMerged - dataptr->previousStats.writeMerged);
                    int dReadTime=  (dataptr->currentStats.readTimeMs - dataptr->previousStats.readTimeMs);
                    int dWriteTime =  (dataptr->currentStats.writeTimeMs - dataptr->previousStats.writeTimeMs);


                    double readRespTime=0.0;
                    if((dReadComplete !=0 || dReadMerged !=0)  ){
                       readRespTime= (dReadTime/(dReadComplete+dReadMerged));
                    }

                    double writeRespTime=0.0;
                    if( (dWriteMerged !=0  || dWriteComplete !=0)){
                        writeRespTime = (dWriteTime/(dWriteComplete+dWriteMerged));
                    }

                    dataptr->averageRWRespTime = (readRespTime+writeRespTime)/2.0;

                    dataptr->previousStats = dataptr->currentStats;


                }
                line = in.readLine().simplified();

            }
        }
        emit data_ready();
        QThread::msleep(m_intervallMs);
        m_cycleTimeMs = cycleTimer.elapsed();
    }
}

void DiskStatReader::update_disk_list()
{
    //find current block device disk it seems it can be dertermined by the existence of device folder in /sys/block/<dev>/device
    QString syspath("/sys/block/");
    QDir block(syspath);
    auto list = block.entryList();
    list.removeFirst(); //remove .
    list.removeFirst(); //remove ..



    for(auto& blkdevice : list ){
        if(QDir(syspath+blkdevice+"/"+"device").exists()){
            //check if disk is already listed in datavecotr
            bool found = false;
            if(find_disk_name_in_data(blkdevice) != nullptr) found = true;

            if(!found){ //disk is not yet part of the datavector
                std::lock_guard<std::mutex> lck(m_DataVecMutex);
                StatTypes::DiskData newDisk;
                newDisk.diskName = blkdevice;
                QFile f(syspath+blkdevice+"/"+"device/type");
                f.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f.isOpen()){
                    QTextStream in(&f);
                    QString line = in.readLine().simplified();
                    switch(line.toInt()){
                    case  0x00 : newDisk.diskType = "Disk"; break;
                    case  0x05 : newDisk.diskType = "Rom"; break;
                    default: newDisk.diskType = "Block Device"; break;
                    }
                }
                else{
                    //nvme does not have the type file
                    newDisk.diskType = "NVMe Disk";
                }

                QString vendor;
                QFile f3(syspath+blkdevice+"/"+"device/vedor");
                f3.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f3.isOpen()){
                    QTextStream in3(&f3);
                    vendor = in3.readLine();
                    f3.close();
                }

                QString model;
                QFile f4(syspath+blkdevice+"/"+"device/vedor");
                f4.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f4.isOpen()){
                    QTextStream in(&f4);
                    vendor = in.readLine();
                    f4.close();
                }

                newDisk.manufacturer = vendor + " "+ model;



                QFile f5(syspath+blkdevice+"/"+"size");
                f5.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f5.isOpen()){
                    QTextStream in(&f5);
                    newDisk.sizeByte = (in.readLine().simplified()).toLongLong()*512; //times linxu blocksize
                }


                newDisk.currentReadKiBsData.resize(m_dataPointsPerMinute,0);
                newDisk.currentActivityData.resize(m_dataPointsPerMinute,0);
                newDisk.currentWriteKiBsData.resize(m_dataPointsPerMinute,0);
                m_DataVec.push_back(std::move(newDisk));

            }

        }
    }


    //todo remove disks that has been removed

}

StatTypes::DiskData *DiskStatReader::find_disk_name_in_data(const QString& name)
{
    for(auto& diskdata : m_DataVec){
        if(diskdata.diskName == name) return &diskdata;
    }
    return nullptr;
}



