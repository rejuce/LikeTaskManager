// Unit tests for RamStatReader (ramstatreader.h/.cpp).
// Verifies /proc/meminfo MemTotal parsing, device initialisation and
// data-vector sizing.
#include <QtTest>
#include <QFile>
#include <QTextStream>
#include <thread>

#include "ramstatreader.h"

class TestRamReader : public QObject
{
    Q_OBJECT
private slots:
    void single_device_named_genmemory();
    void deques_sized_to_one_minute();
    void ram_size_parsed_from_proc();
    void ram_size_positive();
    void measure_main_loop_updates_usage();

private:
    static int mem_total_kib_from_proc();
};

int TestRamReader::mem_total_kib_from_proc()
{
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;
    QTextStream in(&f);
    QString line = in.readLine();
    while (!line.isNull()) {
        if (line.startsWith("MemTotal")) {
            auto parts = line.split(" ");
            return parts.at(parts.size() - 2).toInt();
        }
        line = in.readLine();
    }
    return -1;
}

void TestRamReader::single_device_named_genmemory()
{
    RamStatReader reader;
    QCOMPARE(reader.getDeviceCount(), 1);
    QCOMPARE(reader.m_DataVec[0].AdapterName, QString("GenMemory"));
}

void TestRamReader::deques_sized_to_one_minute()
{
    RamStatReader reader;
    QCOMPARE(reader.m_DataVec[0].currentUsage.size(),
             static_cast<size_t>(StatReader<StatTypes::RamData>::m_dataPointsPerMinute));
    QCOMPARE(reader.m_DataVec[0].currentSwap.size(),
             static_cast<size_t>(StatReader<StatTypes::RamData>::m_dataPointsPerMinute));
}

void TestRamReader::ram_size_parsed_from_proc()
{
    int expected = mem_total_kib_from_proc();
    QVERIFY2(expected > 0, "MemTotal not readable from /proc/meminfo");
    RamStatReader reader;
    QCOMPARE(reader.m_DataVec[0].RamSizeKiB, expected);
}

void TestRamReader::ram_size_positive()
{
    RamStatReader reader;
    QVERIFY(reader.m_DataVec[0].RamSizeKiB > 0);
}

void TestRamReader::measure_main_loop_updates_usage()
{
    // The measurement loop reads live /proc/meminfo. Run it on its own
    // thread for a short while, then check that the deque size is stable
    // and that at least one sample landed in a sane range.
    RamStatReader reader;
    std::thread t([&reader]() { reader.measure_main_loop(); });
    QThread::msleep(600);
    reader.m_quit = true;
    t.join();

    const auto& usage = reader.m_DataVec[0].currentUsage;
    // Deque size must stay at m_dataPointsPerMinute (600).
    QCOMPARE(usage.size(), static_cast<size_t>(600));

    // After ~6 samples the last ~10 elements should be real measurements,
    // not the initial zeros. At least one should be non-zero.
    bool found_nonzero = false;
    for (auto it = usage.end() - 10; it != usage.end(); ++it)
        if (*it != 0.0)
            found_nonzero = true;
    QVERIFY2(found_nonzero, "no non-zero usage sample found after 600 ms");
}

QTEST_MAIN(TestRamReader)
#include "t_ramreader.moc"
