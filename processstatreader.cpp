#include "processstatreader.h"
#include <QDir>
#include <QSet>
#include <mutex>
#include <QTextStream>
#include <QThread>
#include <QElapsedTimer>
#include <unistd.h>
#include <pwd.h>
#include <thread>
#include <iostream>
#include <QRegularExpression>

static ProcessStatReader* currentStatReader = nullptr;

void nethogsCallBack(int action, NethogsMonitorRecord const *data){

    auto* entry = currentStatReader->find_pid_in_data(data->pid);

    if(entry!=nullptr){
        entry->currentReadKiBsNetData = data->recv_kbs;
        entry->currentWriteKiBsNetData = data->sent_kbs;
    }

}


ProcessStatReader::ProcessStatReader(QObject *parent) : QObject(parent), StatReader("ProcessStatReader")
{
    currentStatReader = this;

    update_process_list();

    nethogsT = std::thread([&](){
        int status = nethogsmonitor_loop(nethogsCallBack,nullptr, 250);
        switch(status){
        case NETHOGS_STATUS_OK  : std::cout << "Nethogs Processing Loop OK" << std::endl; break;
        case NETHOGS_STATUS_FAILURE   : std::cerr << "Nethogs Processing Loop FAILURE" << std::endl;  noRootPriv=true; break;
        case NETHOGS_STATUS_NO_DEVICE    : std::cerr << "Nethogs Processing Loop NO DEVICE" << std::endl; noEthDeviceErr=true;  break;
        default : std::cout << "Nethogs Unknown Return Code" << std::endl;
        }
    });


}

ProcessStatReader::~ProcessStatReader()
{
    nethogsmonitor_breakloop();
    if(nethogsT.joinable()) nethogsT.join();
}


void ProcessStatReader::measure_main_loop()
{
    QElapsedTimer cycleTimer;

    // throw std::runtime_error("testexception from process stat reader");

    while(!m_quit){
        m_cycleTimeMs = cycleTimer.elapsed();

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


                        if(line.startsWith("VmRSS")){
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

                    //auto stringlist = line.split(" ");
                    QStringList stringlist = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    precord.currentStats.cpuTimeSec =
                            (stringlist.at(13).toDouble()
                             +stringlist.at(14).toDouble()
                           //  +stringlist.at(15).toDouble()
                            // +stringlist.at(16).toDouble()
                                                       )/static_cast<double>(sysconf(_SC_CLK_TCK));

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
                            precord.currentStats.read_bytes = stringlist.at(1).toLongLong(); //* 9.53674E-7;//9.3132257461548E-7;
                            //auto stringlistBase = model.split(" @ ");

                        }

                        if(line.startsWith("write_bytes")){
                            auto stringlist = line.split(" ");
                            precord.currentStats.write_bytes = stringlist.at(1).toLongLong(); //* 9.53674E-7;//9.3132257461548E-7;
                            //auto stringlistBase = model.split(" @ ");
                            break;
                        }


                        line = in.readLine().simplified();
                    }
                    f.close();
                }

                if(precord.firstIteration) {precord.previousStats = precord.currentStats; precord.firstIteration=false;}

                precord.currentCPUPct =
                    20* ((precord.currentStats.cpuTimeSec - precord.previousStats.cpuTimeSec) /
                                               ((m_cycleTimeMs / 1000.0))) /  (double)sysconf(_SC_NPROCESSORS_CONF);




                precord.currentReadKiBsDiskData = (precord.currentStats.read_bytes-precord.previousStats.read_bytes)/(double)m_cycleTimeMs/1024.0*1000;
                precord.currentWriteKiBsDiskData = (precord.currentStats.write_bytes-precord.previousStats.write_bytes)/(double)m_cycleTimeMs/1024*1000;





                precord.previousStats = precord.currentStats;


            }
        }



        emit data_ready();
        QThread::msleep(m_widgetDataModulus*m_intervallMs);

    }
}

void ProcessStatReader::update_process_list()
{
    QString syspath("/proc");
    QDir pids(syspath);
    auto list = pids.entryList();
    list.removeFirst(); //remove .
    list.removeFirst(); //remove ..

    // Build the set of PIDs currently in /proc
    QSet<int> livePids;
    for(const auto& entry : list){
        bool isNumber = false;
        int pid = entry.toInt(&isNumber, 10);
        if(isNumber) livePids.insert(pid);
    }

    // Remove entries that no longer exist in /proc
    {
        std::lock_guard<std::mutex> lck(m_DataVecMutex);
        for(int i = m_DataVec.size()-1; i >= 0; i--){
            if(!livePids.contains(m_DataVec[i].pid))
                m_DataVec.removeAt(i);
        }
    }

    // Add new PIDs not yet in m_DataVec
    for(int i=0; i<list.size(); i++){
        bool isNumber=false;
        int pid = list[i].toInt(&isNumber,10);
        if(!isNumber) continue;

        if(find_pid_in_data(pid) != nullptr) continue; // already tracked

        StatTypes::ProcessData record;
        record.pid = pid;

        QFile f("/proc/" + list[i] + "/status");
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        if(f.isOpen()){
            QTextStream in(&f);
            QString line = in.readLine().simplified();
            while(!line.isNull()){
                if(line.startsWith("Name")){
                    auto stringlist = line.split(' ', Qt::SkipEmptyParts);
                    if(stringlist.size() >= 2)
                        record.name = stringlist.at(1);
                }
                if(line.startsWith("Uid")){
                    auto stringlist = line.split(' ', Qt::SkipEmptyParts);
                    if(stringlist.size() >= 2){
                        auto* user = getpwuid(stringlist.at(1).toUInt());
                        if(user)
                            record.user = QString::fromLatin1(user->pw_name);
                        else
                            record.user = QString::number(stringlist.at(1).toUInt());
                    }
                    break;
                }
                line = in.readLine().simplified();
            }
            f.close();

            std::lock_guard<std::mutex> lck(m_DataVecMutex);
            m_DataVec.push_back(std::move(record));
        }
    }
}

StatTypes::ProcessData *ProcessStatReader::find_pid_in_data(int pid)
{
    for(auto& pdata : m_DataVec){
        if(pdata.pid == pid) return &pdata;
    }
    return nullptr;
}



