#include "diskstatreader.h"
#include <QDir>
#include <mutex>
#include <QTextStream>


DiskStatReader::DiskStatReader(QObject *parent) : QObject(parent)
{
    update_disk_list();

}


void DiskStatReader::measure_main_loop()
{

while(!m_quit){
    QFile f("/proc/meminfo");
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



            }
            line = in.readLine().simplified();

        }
    }

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
                newDisk.currentReadKBsData.resize(m_dataPointsPerMinute,0);
                newDisk.currentActivityData.resize(m_dataPointsPerMinute,0);
                newDisk.currentWriteKBsData.resize(m_dataPointsPerMinute,0);
                m_DataVec.push_back(std::move(newDisk));

            }

        }
    }


    //todo remove disks that has been removed

}

StatTypes::DiskData *DiskStatReader::find_disk_name_in_data(QString name)
{
    for(auto& diskdata : m_DataVec){
        if(diskdata.diskName == name) return &diskdata;
    }
    return nullptr;
}



