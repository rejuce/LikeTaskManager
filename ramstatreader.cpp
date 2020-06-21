#include "ramstatreader.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <mutex>

RamStatReader::RamStatReader(QObject *parent) : QObject(parent), StatReader("RamStatReader")
{


    m_DataVec.append(StatTypes::RamData());

    m_DataVec[0].AdapterName = "GenMemory";

    m_DataVec[0].currentSwap.resize(m_dataPointsPerMinute,0);
    m_DataVec[0].currentUsage.resize(m_dataPointsPerMinute,0);


    QFile f("/proc/meminfo");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    if(f.isOpen()){
        QTextStream in(&f);

        QString line = in.readLine();
        while(!line.isNull()){

            if(line.startsWith("MemTotal")){
                auto stringlist = line.split(" ");
                m_DataVec[0].RamSizeKiB = stringlist.at(stringlist.size()-2).toInt(); //* 9.53674E-7;//9.3132257461548E-7;
                break;

            }
            line = in.readLine();

        }
    }
}

void RamStatReader::measure_main_loop()
{

    while(!m_quit){
        QFile f("/proc/meminfo");
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        if(f.isOpen()){
            QTextStream in(&f);

            QString line = in.readLine();
            while(!line.isNull()){
                std::lock_guard<std::mutex> lck(m_DataVecMutex);
                if(line.startsWith("MemAvailable")){
                    auto stringlist = line.split(" ");
                    m_DataVec[0].currentUsage.pop_front();
                    m_DataVec[0].currentUsage.push_back(100*(m_DataVec[0].RamSizeKiB-stringlist.at(stringlist.size()-2).toInt())/m_DataVec[0].RamSizeKiB); //* 9.53674E-7;//9.3132257461548E-7;
                }

                if(line.startsWith("SwapTotal")){
                    auto stringlist = line.split(" ");
                    m_DataVec[0].SwapSizeKiB =  stringlist.at(stringlist.size()-2).toInt(); //* 9.53674E-7;//9.3132257461548E-7;
                }

                if(line.startsWith("SwapFree")){
                    auto stringlist = line.split(" ");
                    m_DataVec[0].currentSwap.pop_front();
                    m_DataVec[0].currentSwap.push_back(100*(m_DataVec[0].SwapSizeKiB-stringlist.at(stringlist.size()-2).toInt())/m_DataVec[0].SwapSizeKiB); //* 9.53674E-7;//9.3132257461548E-7;
                    break;
                }




                line = in.readLine();

            }



        }

        filter_data_deque(m_DataVec[0].currentUsage);
        filter_data_deque(m_DataVec[0].currentSwap);

        emit data_ready();

        QThread::msleep(100);
    }

}
