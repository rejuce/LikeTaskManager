#ifndef DISKSTATREADER_H
#define DISKSTATREADER_H

#include <string>
#include <vector>
#include <thread>

class diskStatReader
{
public:
    struct diskData{
        inline diskData() : currentActivityData(dataPointsPerMinute,0.0),currentTransferRateData(dataPointsPerMinute,0.0){}
        std::string diskName;                       //!< name of the disk, exmp /dev/sda
        std::vector<double> currentActivityData;        //!< vector that always holds one minute of datapoints of disk activity data, constant ammoutn of element
        std::vector<double> currentTransferRateData;    //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of element

    };


    diskStatReader();                               //!< cosntructor, intialises the class, should determine how many disk there are etc...

    diskData& getDiskInfo(int diskIndex);           //!< gives a reference to the a specified disk
    int getDiskCount();                             //!< returns how many disks are monitored

    void measure_main_loop();                       //!< collects data for all disk every 100ms, runs until m_quit ist true
    void start();                                   //!< starts the measurment thread
    void stop();                                    //!< stops the measurment thread



private:
    constexpr static int dataPointsPerMinute=600;  //wir machen das abtastinterval nicht varibel, wir wollen alle 100ms eine Messung die 1 Minute ergibt
    std::vector<diskData> m_diskDataVec;            //!< one element for each disk monitored
    double averageReplyDelay=0.0;                   //!< gives the average reply time of the disk
    bool m_quit=false;                               //!< once true, the measurement loop stops
    std::thread m_measureT;                         //!< thread object that executes the measure_main_loop
};

#endif // DISKSTATREADER_H
