#ifndef CPUSTATREADER_H
#define CPUSTATREADER_H

#include <deque>
#include <string>
#include <cpustats.h>
#include "StatReader.h"
#include <QObject>
#include <QString>

namespace StatTypes{
struct CPUData{
  //  inline CPUData(int dataPointsPerMinute) : currentActivityData(dataPointsPerMinute,0.0),currentClockSpeed(dataPointsPerMinute,0.0){}
            std::string CPUName;                            //!< name of the CPU, exmp CPUAll, CPU0,..
            std::deque<double> currentActivityData;        //!< vector that always holds one minute of datapoints of cpu activity data, constant ammoutn of element
            std::deque<double> currentClockSpeed;          //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of elemen

};

}



class CPUStatReader : public QObject, public StatReader<StatTypes::CPUData>
{
    Q_OBJECT

public:
    CPUStatReader();

    void measure_main_loop();   //!< implements the main measurement loop that runs contionously untils stopped in a separate thread


    friend class LTM;
    QString CPUCoresPhys;
    QString CPUCoresLogical;
    QString cacheSize;
    QString model;


signals:
    void data_ready();

};

//Q_DECLARE_METATYPE(QVector<StatTypes::CPUData>);
//Q_DECLARE_METATYPE(std::mutex);

#endif // CPUSTATREADER_H
