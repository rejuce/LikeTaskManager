#ifndef NETWORKSTATREADER_H
#define NETWORKSTATREADER_H

#include <QObject>
#include <deque>
#include "StatReader.h"
#include <QNetworkInterface>

namespace StatTypes{

struct WifiLinkStats{
    QString MBs="Unknown";
    QString dB="Unknown";
    QString noise="Unknown";
};

struct EthernetLinkStats{
    QString LinkSpeed="Unknown";
    QString LinkMode="Unknown";
};


struct NetworkData{
    //  inline CPUData(int dataPointsPerMinute) : currentActivityData(dataPointsPerMinute,0.0),currentClockSpeed(dataPointsPerMinute,0.0){}
    QString AdapterName = " ";                     //!< name of the CPU, exmp CPUAll, CPU0,..
    std::deque<double> currentTxData;        //!< vector that always holds one minute of datapoints of cpu activity data, constant ammoutn of element
    std::deque<double> currentRxData;        //!< vector that always holds one minute of datapoints of transfer statitisc data, constant ammoutn of elemen
    qint64 previousTxTotalBytes=0;              //!<as only the total amoutn of bytes can be read, the rate must be calcualted through the two readouts
    qint64 previousRxTotalBytes=0;              //!<as only the total amoutn of bytes can be read, the rate must be calcualted through the two readouts
    QNetworkInterface::InterfaceType interfaceType = QNetworkInterface::InterfaceType::Unknown;  //!< describes if can, wifi, ehtnertnet etc
    QStringList ip4Addresses;
    QStringList ip6Addresses;
    QString hwAddress = "N/A";
    QString speedLinkInfo = "N/A";
};

}

class NetworkStatReader : public QObject, public StatReader<StatTypes::NetworkData>
{
    Q_OBJECT
public:
    NetworkStatReader();



    void measure_main_loop();   //!< implements the main measurement loop that runs contionously untils stopped in a separate thread



    friend class LTM;

signals:
    void data_ready();


private:
    qint64 read_total_rx_bytes(QString AdapterName);
    qint64 read_total_tx_bytes(QString AdapterName);
    StatTypes::WifiLinkStats read_wifi_stats(QString AdapterName);
    StatTypes::EthernetLinkStats read_ethernet_link_stats(QString AdapterName);

    void update_static_data(StatTypes::NetworkData& iface);

};

#endif // NETWORKSTATREADER_H
