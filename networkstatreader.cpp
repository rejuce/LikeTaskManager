#include "networkstatreader.h"
#include <QDir>
#include <QTextStream>
#include <QThread>
#include <QNetworkInterface>

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
        static int  slowcounter =0;

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

                if(slowcounter%m_staticDataModulus==0) update_static_data(iface);


            }
        }

        emit data_ready();

        QThread::msleep(100);
        slowcounter++;
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

StatTypes::WifiLinkStats NetworkStatReader::read_wifi_stats(QString AdapterName)
{
    StatTypes::WifiLinkStats ret;

    QFile f("/proc/net/wireless");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    if(f.isOpen()){
        QTextStream in(&f);

        QString line = in.readLine();
       while(!line.isNull()){

            if(line.startsWith(AdapterName)){
                auto stringlist = line.split("  ");
                ret.MBs = stringlist.at(1);
                ret.MBs.chop(1);
                ret.dB = stringlist.at(2);
                ret.dB.chop(1);
                ret.noise = stringlist.at(3);

            }
           line = in.readLine();
           line.remove(0,2); //somehow the line with dapater name starts with space...
        }
    }
    return ret;
}

StatTypes::EthernetLinkStats NetworkStatReader::read_ethernet_link_stats(QString AdapterName){
    StatTypes::EthernetLinkStats ret;

    QFile f("/sys/class/net/"+ AdapterName +"/speed");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&f);
    //int speed=0;
    ret.LinkSpeed = in.readLine();

    QFile f2("/sys/class/net/"+ AdapterName +"/duplex");
    f2.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in2(&f2);
    //int speed=0;
    ret.LinkMode = in2.readLine();

    return ret;

}

void NetworkStatReader::update_static_data(StatTypes::NetworkData& iface){


    QNetworkInterface QtInterface = QNetworkInterface::interfaceFromName(iface.AdapterName);

    iface.interfaceType = QtInterface.type();
    if(iface.interfaceType == QNetworkInterface::InterfaceType::Ethernet){
        auto ethLinkStat = read_ethernet_link_stats(iface.AdapterName);
        iface.speedLinkInfo = ethLinkStat.LinkSpeed + " / " + ethLinkStat.LinkMode;
    }
    else if(iface.interfaceType == QNetworkInterface::InterfaceType::Wifi){

        auto stats = read_wifi_stats(iface.AdapterName);
        iface.speedLinkInfo = stats.MBs + QString(" MB/s  SingalLv: ") + stats.dB + QString(" dB  Noise: "+ stats.noise );
    }


    iface.hwAddress= QtInterface.hardwareAddress();

    auto addressList = QtInterface.addressEntries();

    iface.ip4Addresses.clear();
    iface.ip6Addresses.clear();

    for(auto& networkAddressEntry : addressList){
        auto currentIPadr = networkAddressEntry.ip();
        QPair<QHostAddress, int> curentIP = QHostAddress::parseSubnet(currentIPadr.toString()+"/"+networkAddressEntry.netmask().toString());

        if(currentIPadr.protocol()==QAbstractSocket::IPv4Protocol){
            iface.ip4Addresses.push_back(currentIPadr.toString()+"/"+QString::number(curentIP.second) );
        }
        else if (currentIPadr.protocol()==QAbstractSocket::IPv6Protocol){
            iface.ip6Addresses.push_back(currentIPadr.toString()+"/"+QString::number(curentIP.second) );
        }

    }





}
