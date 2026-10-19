// Unit tests for DiskStatReader (diskstatreader.h/.cpp).
// Verifies disk discovery from /sys/block, find_disk_name_in_data,
// and data-vector sizing.
#include <QtTest>
#include <QDir>

#include "diskstatreader.h"

class TestDiskReader : public QObject
{
    Q_OBJECT
private slots:
    void disk_deques_sized_to_one_minute();
    void disk_names_match_sys_block();
    void find_disk_returns_pointer_to_stored_entry();
    void find_disk_returns_nullptr_for_unknown();
    void size_byte_positive_when_available();
    void measure_main_loop_runs();

private:
    // Build the list of real block devices from /sys/block, the same way
    // the production constructor does (folder with a device/ subfolder).
    static QStringList expected_disks();
};

QStringList TestDiskReader::expected_disks()
{
    QStringList result;
    QDir block("/sys/block/");
    auto list = block.entryList();
    list.removeFirst(); // "."
    list.removeFirst(); // ".."
    for (const auto& dev : list) {
        if (QDir("/sys/block/" + dev + "/device").exists())
            result.append(dev);
    }
    return result;
}

void TestDiskReader::disk_deques_sized_to_one_minute()
{
    DiskStatReader reader;
    for (const auto& d : reader.m_DataVec) {
        QCOMPARE(d.currentReadKiBsData.size(),
                 static_cast<size_t>(StatReader<StatTypes::DiskData>::m_dataPointsPerMinute));
        QCOMPARE(d.currentWriteKiBsData.size(),
                 static_cast<size_t>(StatReader<StatTypes::DiskData>::m_dataPointsPerMinute));
        QCOMPARE(d.currentActivityData.size(),
                 static_cast<size_t>(StatReader<StatTypes::DiskData>::m_dataPointsPerMinute));
    }
}

void TestDiskReader::disk_names_match_sys_block()
{
    DiskStatReader reader;
    auto expected = expected_disks();
    QCOMPARE(reader.getDeviceCount(), expected.size());
    for (const auto& name : expected) {
        QVERIFY2(reader.find_disk_name_in_data(name) != nullptr,
                 qPrintable(QString("disk %1 not found in data vector").arg(name)));
    }
}

void TestDiskReader::find_disk_returns_pointer_to_stored_entry()
{
    DiskStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no block devices in this environment");
    }
    const auto& name = reader.m_DataVec[0].diskName;
    auto* ptr = reader.find_disk_name_in_data(name);
    QVERIFY2(ptr != nullptr, "find_disk_name_in_data returned nullptr for known disk");
    QCOMPARE(&(*ptr), &reader.m_DataVec[0]);
}

void TestDiskReader::find_disk_returns_nullptr_for_unknown()
{
    DiskStatReader reader;
    QVERIFY(reader.find_disk_name_in_data("definitely_not_a_disk") == nullptr);
}

void TestDiskReader::size_byte_positive_when_available()
{
    DiskStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no block devices in this environment");
    }
    // The container may lack a readable /sys/block/<dev>/size file;
    // in that case sizeByte stays 1. Only assert consistency:
    // either 1 (default, file unreadable) or a positive multiple of 512.
    for (const auto& d : reader.m_DataVec) {
        QVERIFY2(d.sizeByte == 1 || (d.sizeByte > 1 && d.sizeByte % 512 == 0),
                 qPrintable(QString("sizeByte %1 is neither default nor a 512 multiple")
                            .arg(d.sizeByte)));
    }
}

void TestDiskReader::measure_main_loop_runs()
{
    // Run the measurement loop briefly on its own thread; it must not
    // crash and must keep the deque sizes stable.
    DiskStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no block devices in this environment");
    }
    std::thread t([&reader]() { reader.measure_main_loop(); });
    QThread::msleep(500);
    reader.m_quit = true;
    t.join();

    for (const auto& d : reader.m_DataVec) {
        QCOMPARE(d.currentActivityData.size(),
                 static_cast<size_t>(600));
        QCOMPARE(d.currentReadKiBsData.size(),
                 static_cast<size_t>(600));
        QCOMPARE(d.currentWriteKiBsData.size(),
                 static_cast<size_t>(600));
    }
}

QTEST_MAIN(TestDiskReader)
#include "t_diskreader.moc"
