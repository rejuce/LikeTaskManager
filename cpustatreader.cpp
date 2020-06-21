#include "cpustatreader.h"
#include "StatReader.h"
#include <QThread>
#include <QFile>
#include <QTextStream>

CPUStatReader::CPUStatReader():  StatReader("CPUStatReader")  {

    CPUStats::get_cpus_activity();//dummy read to populate
    CPUStats::get_cpus_activity();//dummy read to populate

    auto currentStat= CPUStats::get_cpus_activity();


    for(size_t i=0; i<currentStat.size(); i++){
        StatTypes::CPUData record;
        record.CPUName = currentStat[i].first;  //intialise names
        record.currentActivityData.resize(m_dataPointsPerMinute,0);
        record.currentClockSpeed.resize(m_dataPointsPerMinute,0);
        m_DataVec.append(std::move(record));
    }


    QFile f("/proc/cpuinfo");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    if(f.isOpen()){
        QTextStream in(&f);

        QString line = in.readLine();
        while(!line.isNull()){

            if(line.startsWith("model")){
                auto stringlist = line.split("\t: ");
                model = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
                //auto stringlistBase = model.split(" @ ");


            }


            if(line.startsWith("cache")){
                auto stringlist = line.split("\t: ");
                cacheSize = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
            }

            if(line.startsWith("siblings")){
                auto stringlist = line.split("\t: ");
                CPUCoresLogical = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
            }

            if(line.startsWith("cpu cores")){
                auto stringlist = line.split("\t: ");
                CPUCoresPhys = stringlist.at(1); //* 9.53674E-7;//9.3132257461548E-7;
                break;
            }



            line = in.readLine();

        }
    }


}

void CPUStatReader::measure_main_loop()
{

    while(!m_quit){


        auto currentCPU = CPUStats::get_cpus_activity_vector();

        {//enterign data race critical section

            std::lock_guard<std::mutex> lock(m_DataVecMutex);

            for(size_t i=0; i<m_DataVec.size(); i++){
                m_DataVec[i].currentActivityData.pop_front();
                m_DataVec[i].currentActivityData.push_back(currentCPU[i]);

                //filtering first order
                //for(int k = +1; k < cpuYPlotData[i].size()-1; k++)
                //int k = cpuYPlotData[i].size()-2;
                //     cpuYPlotData[i][k] = (cpuYPlotData[i][k-1]+ cpuYPlotData[i][k] +cpuYPlotData[i][k+1] )/ 3.0;

                //filtering 2nd order to power n
                size_t k = m_DataVec[i].currentActivityData.size()-3;
                for(int n=8; n>=0; n--){
                    //for(int k = +2; k < cpuYPlotData[i].size()-2; k++)
                    m_DataVec[i].currentActivityData[k] = (m_DataVec[i].currentActivityData[k-2] + m_DataVec[i].currentActivityData[k-1] + m_DataVec[i].currentActivityData[k] +m_DataVec[i].currentActivityData[k+1] +m_DataVec[i].currentActivityData[k+2] )/ 5.0;
                    k--;
                }



                if(i==0){//only measure average over all cores
                    QFile f("/proc/cpuinfo");
                    f.open(QIODevice::ReadOnly | QIODevice::Text);
                    if(f.isOpen()){
                        QTextStream in(&f);

                        QString line = in.readLine();
                        while(!line.isNull()){

                            if(line.startsWith("cpu MHz")){
                                auto stringlist = line.split("\t: ");
                                m_DataVec[0].currentClockSpeed.pop_front();
                                m_DataVec[0].currentClockSpeed.push_back(stringlist.at(1).toDouble());
                                filter_data_deque( m_DataVec[0].currentClockSpeed);


                                break;

                            }


                            line = in.readLine();

                        }
                    }
                }
            }

        }

        emit data_ready();

        QThread::msleep(100);






    }

}
