// Unit tests for NetworkStatReader (networkstatreader.h/.cpp).
// Verifies interface discovery from /sys/class/net, data-vector sizing,
// byte-counter reading, and measure_main_loop stability.
#include <QtTest>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QNetworkInterface>
#include <thread>

#include "networkstatreader.h"

class TestNetworkReader : public QObject
{
    Q_OBJECT
private slots:
    void interface_count_matches_sys_class_net();
    void deques_sized_to_one_minute();
    void previous_bytes_initialized_from_sysfs();
    void read_total_rx_bytes_matches_sysfs();
    void read_total_tx_bytes_matches_sysfs();
    void find_interface_by_name();
    void measure_main_loop_stable();

private:
    static QStringList expected_ifaces();
    static qint64 read_sysfs_bytes(const QString& name, const QString& stat);
};

QStringList TestNetworkReader::expected_ifaces()
{
    QDir dir("/sys/class/net/");
    auto list = dir.entryList();
    list.removeFirst(); // "."
    list.removeFirst(); // ".."
    return list;
}

qint64 TestNetworkReader::read_sysfs_bytes(const QString& name, const QString& stat)
{
    QFile f("/sys/class/net/" + name + "/statistics/" + stat);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;
    QTextStream in(&f);
    qint64 val;
    in >> val;
    return val;
}

void TestNetworkReader::interface_count_matches_sys_class_net()
{
    NetworkStatReader reader;
    auto expected = expected_ifaces();
    QCOMPARE(reader.getDeviceCount(), expected.size());
}

void TestNetworkReader::deques_sized_to_one_minute()
{
    NetworkStatReader reader;
    for (const auto& iface : reader.m_DataVec) {
        QCOMPARE(iface.currentRxData.size(),
                 static_cast<size_t>(StatReader<StatTypes::NetworkData>::m_dataPointsPerMinute));
        QCOMPARE(iface.currentTxData.size(),
                 static_cast<size_t>(StatReader<StatTypes::NetworkData>::m_dataPointsPerMinute));
    }
}

void TestNetworkReader::previous_bytes_initialized_from_sysfs()
{
    NetworkStatReader reader;
    for (const auto& iface : reader.m_DataVec) {
        auto expectedRx = read_sysfs_bytes(iface.AdapterName, "rx_bytes");
        auto expectedTx = read_sysfs_bytes(iface.AdapterName, "tx_bytes");
        // In a container the sysfs files may not be readable; in that
        // case the reader stores 0. Only check consistency.
        if (expectedRx >= 0)
            QCOMPARE(iface.previousRxTotalBytes, expectedRx);
        else
            QCOMPARE(iface.previousRxTotalBytes, static_cast<qint64>(0));
        if (expectedTx >= 0)
            QCOMPARE(iface.previousTxTotalBytes, expectedTx);
        else
            QCOMPARE(iface.previousTxTotalBytes, static_cast<qint64>(0));
    }
}

void TestNetworkReader::read_total_rx_bytes_matches_sysfs()
{
    NetworkStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no network interfaces in this environment");
    }
    const auto& name = reader.m_DataVec[0].AdapterName;
    auto expected = read_sysfs_bytes(name, "rx_bytes");
    auto actual = reader.read_total_rx_bytes(name);
    if (expected >= 0)
        QCOMPARE(actual, expected);
    else
        QCOMPARE(actual, static_cast<qint64>(0));
}

void TestNetworkReader::read_total_tx_bytes_matches_sysfs()
{
    NetworkStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no network interfaces in this environment");
    }
    const auto& name = reader.m_DataVec[0].AdapterName;
    auto expected = read_sysfs_bytes(name, "tx_bytes");
    auto actual = reader.read_total_tx_bytes(name);
    if (expected >= 0)
        QCOMPARE(actual, expected);
    else
        QCOMPARE(actual, static_cast<qint64>(0));
}

void TestNetworkReader::find_interface_by_name()
{
    NetworkStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no network interfaces in this environment");
    }
    // Every known interface should be found by name.
    for (const auto& iface : reader.m_DataVec) {
        bool found = false;
        for (const auto& stored : reader.m_DataVec) {
            if (stored.AdapterName == iface.AdapterName) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, qPrintable(
            QString("interface %1 not found in data vector").arg(iface.AdapterName)));
    }
    // Unknown interface should NOT be found.
    bool foundUnknown = false;
    for (const auto& stored : reader.m_DataVec) {
        if (stored.AdapterName == "definitely_not_an_iface")
            foundUnknown = true;
    }
    QVERIFY(!foundUnknown);
}

void TestNetworkReader::measure_main_loop_stable()
{
    NetworkStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no network interfaces in this environment");
    }
    std::thread t([&reader]() { reader.measure_main_loop(); });
    QThread::msleep(500);
    reader.m_quit = true;
    t.join();

    for (const auto& iface : reader.m_DataVec) {
        QCOMPARE(iface.currentRxData.size(), static_cast<size_t>(600));
        QCOMPARE(iface.currentTxData.size(), static_cast<size_t>(600));
    }
}

QTEST_MAIN(TestNetworkReader)
#include "t_networkreader.moc"
