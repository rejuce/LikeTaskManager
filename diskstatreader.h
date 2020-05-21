#ifndef DISKSTATREADER_H
#define DISKSTATREADER_H

#include <string>
#include <deque>
#include <QObject>
#include <QString>
#include "StatReader.h"


namespace StatTypes{

struct rawDiskStat{
    int secRead=0;
    int secWrite=0;
    int IOTime=0;
    int readsComplete=0;
    int readsMerged=0;
    int readTimeMs=0;
    int writeComplete=0;
    int writeMerged=0;
    int writeTimeMs=0;
};

    struct DiskData{
        QString diskName;                       //!< name of the disk, exmp /dev/sda
        QString diskType;
        QString manufacturer;
        qint64 sizeByte;
        std::deque<double> currentActivityData;        //!< vector that always holds one minute of datapoints of disk activity data, constant ammoutn of element
        std::deque<double> currentReadKiBsData;    //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of element
        std::deque<double> currentWriteKiBsData;    //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of element
        double averageRWRespTime=0.0;
        double currentIOPS=0.0;                         //!< some of all completed read/write requests in one intervall

        //https://www.percona.com/doc/percona-toolkit/2.1/pt-diskstats.html
        rawDiskStat previousStats;              //from previous time timepoint
        rawDiskStat currentStats;
        bool firstIteration = true;  // must be used to ensure no overflow on first timeinterval


       };



}

class DiskStatReader: public QObject, public StatReader<StatTypes::DiskData>
{
    Q_OBJECT

public:



    DiskStatReader(QObject *parent = nullptr);                               //!< cosntructor, intialises the class, should determine how many disk there are etc...

   void measure_main_loop();                       //!< collects data for all disk every 100ms, runs until m_quit ist true


signals:
    void data_ready();

private:
    void update_disk_list();
    StatTypes::DiskData* find_disk_name_in_data(QString name);  //!< looks for supplied disk name in data vector, if not foudn return nullptr


};

#endif // DISKSTATREADER_H
