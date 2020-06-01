#include "processstatreader.h"
#include <QDir>
#include <mutex>
#include <QTextStream>
#include <QThread>
#include <QElapsedTimer>
#include <unistd.h>

ProcessStatReader::ProcessStatReader(QObject *parent) : QObject(parent)
{
    update_process_list();

}


void ProcessStatReader::measure_main_loop()
{
    QElapsedTimer cycleTimer;

    while(!m_quit){
     m_cycleTimeMs = cycleTimer.elapsed()-(m_widgetDataModulus*m_intervallMs);

        cycleTimer.restart();

        update_process_list();

        {
            std::lock_guard<std::mutex> lck(m_DataVecMutex);
            for(auto& precord : m_DataVec){

                QFile f("/proc/" + QString::number(precord.pid) +"/status");
                f.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f.isOpen()){
                    QTextStream in(&f);

                    QString line = in.readLine().simplified();
                    while(!line.isNull()){


                        if(line.startsWith("VmSize")){
                            auto stringlist = line.split(" ");
                            precord.currentMemKiB = stringlist.at(1).toInt(); //* 9.53674E-7;//9.3132257461548E-7;
                            //auto stringlistBase = model.split(" @ ");
                            break;
                        }
                        line = in.readLine().simplified();
                    }
                    f.close();
                }


                f.setFileName("/proc/" + QString::number(precord.pid) +"/stat");
                f.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f.isOpen()){
                    QTextStream in(&f);

                    QString line = in.readLine().simplified();

                    auto stringlist = line.split(" ");
                    precord.currentStats.cpuTimeSec =
                            (stringlist.at(13).toInt()
                             +stringlist.at(14).toInt()
                             +stringlist.at(15).toInt()
                             +stringlist.at(16).toInt())/(double)sysconf(_SC_CLK_TCK);

                    f.close();
                }



                f.setFileName("/proc/" + QString::number(precord.pid) +"/io");
                f.open(QIODevice::ReadOnly | QIODevice::Text);
                if(f.isOpen()){
                    QTextStream in(&f);

                    QString line = in.readLine().simplified();
                    while(!line.isNull()){


                        if(line.startsWith("read_bytes")){
                            auto stringlist = line.split(" ");
                            precord.currentStats.read_bytes = stringlist.at(1).toInt(); //* 9.53674E-7;//9.3132257461548E-7;
                            //auto stringlistBase = model.split(" @ ");

                        }

                        if(line.startsWith("write_bytes")){
                            auto stringlist = line.split(" ");
                            precord.currentStats.write_bytes = stringlist.at(1).toInt(); //* 9.53674E-7;//9.3132257461548E-7;
                            //auto stringlistBase = model.split(" @ ");
                            break;
                        }


                        line = in.readLine().simplified();
                    }
                    f.close();
                }

                if(precord.firstIteration) {precord.previousStats = precord.currentStats; precord.firstIteration=false;}

                precord.currentCPUPct =
                        100*(((precord.currentStats.cpuTimeSec-precord.previousStats.cpuTimeSec)/sysconf(_SC_NPROCESSORS_ONLN))
                             /m_cycleTimeMs);


                precord.currentReadKiBsDiskData = (precord.currentStats.read_bytes-precord.previousStats.read_bytes)/m_cycleTimeMs;
                precord.currentWriteKiBsDiskData = (precord.currentStats.write_bytes-precord.previousStats.write_bytes)/m_cycleTimeMs;




                precord.previousStats = precord.currentStats;


            }
        }



        emit data_ready();
        QThread::msleep(m_widgetDataModulus*m_intervallMs);

    }
}

void ProcessStatReader::update_process_list()
{
    //find current block device disk it seems it can be dertermined by the existence of device folder in /sys/block/<dev>/device
    QString syspath("/proc");
    QDir pids(syspath);
    auto list = pids.entryList();
    list.removeFirst(); //remove .
    list.removeFirst(); //remove ..


    for(int i=0; i<list.size();i++){
        bool isNumber=false;
        int pid = list[i].toInt(&isNumber,10);
        if(!isNumber) {
            list.removeAt(i);
            i--;
        }
        else if(find_pid_in_data(pid)==nullptr) {//not existent yet, add new entry and populate static data members
            StatTypes::ProcessData record;

            record.pid = pid;


            QFile f("/proc/"+list[i] +"/status");
            f.open(QIODevice::ReadOnly | QIODevice::Text);
            if(f.isOpen()){

                QTextStream in(&f);

                QString line = in.readLine().simplified();
                while(!line.isNull()){

                    if(line.startsWith("Name")){
                        auto stringlist = line.split(" ");
                        record.name = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
                        //auto stringlistBase = model.split(" @ ");
                    }

                    if(line.startsWith("UiD")){
                        auto stringlist = line.split(" ");
                        record.user = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
                        //auto stringlistBase = model.split(" @ ");
                        break;
                    }

                    line = in.readLine();
                }
//only add record if there was enough permission to read it
                std::lock_guard<std::mutex> lck(m_DataVecMutex);
                m_DataVec.push_back(std::move(record));

            }



        }
        std::lock_guard<std::mutex> lck(m_DataVecMutex);
        //now check if there are processes listed that are non existent
        bool found=false;
        for(int i=0; i<m_DataVec.size(); i++)
            if(m_DataVec[i].pid == pid) found= true;
        if(!found) m_DataVec.removeAt(i); //entry in adatavec that is not in proc anymore -> remove termindated process




    }






}

StatTypes::ProcessData *ProcessStatReader::find_pid_in_data(int pid)
{
    for(auto& pdata : m_DataVec){
        if(pdata.pid == pid) return &pdata;
    }
    return nullptr;
}



