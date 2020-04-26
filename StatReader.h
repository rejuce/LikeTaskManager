#ifndef STATREADER_H
#define STATREADER_H

#include <thread>
#include <mutex>
#include <vector>
#include <QVector>

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
    QVector<T> m_DataVec;            //!< one element for each device monitored
    std::mutex m_DataVecMutex;
    bool m_quit=false;                   //!< once true, the measurement loop stops
    std::thread m_measureT;              //!< thread object that executes the measure_main_loop
};

#endif // STATREADER_H
