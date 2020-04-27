#include "networkstatreader.h"
#include <QDir>
#include <QTextStream>
#include <QThread>


NetworkStatReader::NetworkStatReader(){

    QDir directory("/sys/class/net/");
    QStringList adapters = directory.entryList();
    adapters.removeFirst(); //remove .
    adapters.removeFirst(); //remove ..


    m_DataVec.resize(adapters.size());

    for(int i=0; i<adapters.size(); i++){
        m_DataVec[i].AdapterName = adapters[i];  //intialise names
        m_DataVec[i].currentRxData.resize(m_dataPointsPerMinute,0);
        m_DataVec[i].currentTxData.resize(m_dataPointsPerMinute,0);
        m_DataVec[i].previousRxTotalBytes = read_total_rx_bytes(adapters[i]);
        m_DataVec[i].previousTxTotalBytes = read_total_tx_bytes(adapters[i]);
    }


}

void NetworkStatReader::measure_main_loop()
{
    while(!m_quit){

        {//enterign data race critical section

            std::lock_guard<std::mutex> lock(m_DataVecMutex);

            for(auto& iface : m_DataVec){

                auto currentRxBytes = read_total_rx_bytes(iface.AdapterName);
                auto currentTxBytes = read_total_tx_bytes(iface.AdapterName);



                iface.currentRxData.pop_front();
                iface.currentTxData.pop_front();

                iface.currentRxData.push_back((currentRxBytes-iface.previousRxTotalBytes)/100.0); //calculat ein kbyte/sec
                iface.currentTxData.push_back(-(currentTxBytes-iface.previousTxTotalBytes)/100.0);

                //filtering first order
                //for(int k = +1; k < cpuYPlotData[i].size()-1; k++)
                //int k = cpuYPlotData[i].size()-2;
                //     cpuYPlotData[i][k] = (cpuYPlotData[i][k-1]+ cpuYPlotData[i][k] +cpuYPlotData[i][k+1] )/ 3.0;

                //filtering 2nd order to power n
                filter_data_deque(iface.currentRxData);
                filter_data_deque(iface.currentTxData);

                iface.previousRxTotalBytes = currentRxBytes;
                iface.previousTxTotalBytes = currentTxBytes;



            }
        }

        emit data_ready();

        QThread::msleep(100);
    }
}

qint64 NetworkStatReader::read_total_rx_bytes(QString AdapterName){
    QFile f("/sys/class/net/"+ AdapterName +"/statistics/rx_bytes");
    f.open(QIODevice::ReadOnly | QIODevice::Text );
    QTextStream in(&f);
     qint64 bytecount=0;
   // QString line = in.readLine();
    in >> bytecount;
    return bytecount;

}

qint64 NetworkStatReader::read_total_tx_bytes(QString AdapterName)
{
    QFile f("/sys/class/net/"+ AdapterName +"/statistics/tx_bytes");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&f);
    qint64 bytecount=0;
    in >> bytecount;
    return bytecount;
}
