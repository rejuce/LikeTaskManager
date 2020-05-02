#ifndef STATREADER_H
#define STATREADER_H

#include <thread>
#include <mutex>
#include <vector>
#include <QVector>
#include <deque>

template<typename T>
class StatReader {
public:


    //StatReader(){};                              //!< cosntructor, intialises the class, should determine how many disk there are etc...
   // virtual ~StatReader(){};


    const T& getStatData(int DeviceIndex){
        return m_DataVec.at(DeviceIndex);
    };                                              //!< gives a reference to the a specified device data entry
    int getDeviceCount(){
        return m_DataVec.size();
    }                                          //!< returns how many disks are monitored

    virtual void measure_main_loop()=0;                       //!< collects data for all disk every 100ms, runs until m_quit ist true

    //default filter algorhytm for all the used data
    void filter_data_deque(std::deque<double>& data){

        size_t k = data.size()-3;
        for(int n=8; n>=0; n--){
            //for(int k = +2; k < cpuYPlotData[i].size()-2; k++)
            data[k] = (data[k-2] + data[k-1] + data[k] +data[k+1] +data[k+2] )/ 5.0;
            k--;

        }
    };


    void start(){
        m_quit=false;
        if(!m_measureT.joinable()){
            m_measureT = std::thread([&](){
            measure_main_loop();
        });
        }
    }                                   //!< starts the measurment thread
    void stop(){ m_quit=false;}                                    //!< stops the measurment thread



protected:
    friend class LTM;
    constexpr static int m_dataPointsPerMinute=600;  //wir machen das abtastinterval nicht varibel, wir wollen alle 100ms eine Messung die 1 Minute ergibt
    constexpr static int m_widgetDataModulus = 5;
    constexpr static int m_staticDataModulus = 50;
    QVector<T> m_DataVec;            //!< one element for each device monitored
    std::mutex m_DataVecMutex;
    bool m_quit=false;                   //!< once true, the measurement loop stops
    std::thread m_measureT;              //!< thread object that executes the measure_main_loop
};

#endif // STATREADER_H
