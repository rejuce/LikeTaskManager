#ifndef DISKSTATREADER_H
#define DISKSTATREADER_H

#include <string>
#include <deque>
#include <QObject>
#include <QString>
#include "StatReader.h"


namespace StatTypes{



    struct DiskData{
        QString diskName;                       //!< name of the disk, exmp /dev/sda
        std::deque<double> currentActivityData;        //!< vector that always holds one minute of datapoints of disk activity data, constant ammoutn of element
        std::deque<double> currentReadKBsData;    //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of element
        std::deque<double> currentWriteKBsData;    //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of element
        double averageReplyDelay=0.0;
    };              //!<hold max size of mounted swap space , also relfects if swapfile gets mounted /unmounted

}

class DiskStatReader: public QObject, public StatReader<StatTypes::DiskData>
{
    Q_OBJECT

public:



    DiskStatReader(QObject *parent = nullptr);                               //!< cosntructor, intialises the class, should determine how many disk there are etc...

   void measure_main_loop();                       //!< collects data for all disk every 100ms, runs until m_quit ist true


signals:
    void data_ready();


};

#endif // DISKSTATREADER_H
