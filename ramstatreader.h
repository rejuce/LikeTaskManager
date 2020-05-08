#ifndef RAMSTATREADER_H
#define RAMSTATREADER_H

#include <QObject>
#include <deque>
#include "StatReader.h"

namespace StatTypes{


struct RamData{
    //  inline CPUData(int dataPointsPerMinute) : currentActivityData(dataPointsPerMinute,0.0),currentClockSpeed(dataPointsPerMinute,0.0){}
    QString AdapterName;                     //!< name of the memory
    std::deque<int> currentUsage;        //!< vector that always holds one minute of datapoints of cpu activity data, constant ammoutn of element
    std::deque<int> currentSwap;        //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of elemen
    int RamSizeKiB = 0;                  //!<hold max size of installed memory
    int SwapSizeKiB = 0;                 //!<hold max size of mounted swap space , also relfects if swapfile gets mounted /unmounted
};
}

class RamStatReader : public QObject, public StatReader<StatTypes::RamData>
{
    Q_OBJECT
public:
    explicit RamStatReader(QObject *parent = nullptr);

     void measure_main_loop();   //!< implements the main measurement loop that runs contionously untils stopped in a separate thread




signals:
    void data_ready();
};

#endif // RAMSTATREADER_H
