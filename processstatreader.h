#ifndef PRSTATREADER_H
#define PRSTATREADER_H

#include <QObject>
#include <QString>
#include "StatReader.h"
#include <libnethogs.h>


namespace StatTypes{

struct ProcessStat{
    int64_t read_bytes=0;           //bytes read from the underlyling block layer
    int64_t write_bytes=0;          //bytes written to the underlying block layer
    double cpuTimeSec=0;

};

    struct ProcessData{
        int pid;                       //!< pid of the process
        QString user;
        double  currentReadKiBsDiskData;
        double  currentWriteKiBsDiskData;
        double  currentReadKiBsNetData;
        double  currentWriteKiBsNetData;

        double  currentMemKiB;
        double  currentCPUPct;
        QString name;

        ProcessStat previousStats;              //from previous time timepoint
        ProcessStat currentStats;
        bool firstIteration = true;  // must be used to ensure no overflow on first timeinterval


       };



}

class ProcessStatReader: public QObject, public StatReader<StatTypes::ProcessData>
{
    Q_OBJECT

public:


    ProcessStatReader(QObject *parent = nullptr);                               //!< cosntructor, intialises the class, should determine how many disk there are etc...
    ~ProcessStatReader();
    bool noEthDeviceErr = false;
    bool noRootPriv = false;

   void measure_main_loop();                       //!< collects data for all disk every 100ms, runs until m_quit ist true


signals:
    void data_ready();

private:
    friend void nethogsCallBack(int action, NethogsMonitorRecord const *data);
    std::thread nethogsT;
    void update_process_list();
    StatTypes::ProcessData* find_pid_in_data(int pid);  //!< looks for supplied disk name in data vector, if not foudn return nullptr



};

#endif // DISKSTATREADER_H
