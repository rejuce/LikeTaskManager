#include "cpustatreader.h"
#include "StatReader.h"
#include <QThread>

CPUStatReader::CPUStatReader()  {

    CPUStats::get_cpus_activity();//dummy read to populate
    CPUStats::get_cpus_activity();//dummy read to populate

    auto currentStat= CPUStats::get_cpus_activity();
    m_DataVec.resize(currentStat.size());

    for(size_t i=0; i<currentStat.size(); i++){
        m_DataVec[i].CPUName = currentStat[i].first;  //intialise names
        m_DataVec[i].currentActivityData.resize(m_dataPointsPerMinute,0);
        m_DataVec[i].currentClockSpeed.resize(m_dataPointsPerMinute,0);
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
            }
        }

        emit data_ready();

        QThread::msleep(100);






    }

}
