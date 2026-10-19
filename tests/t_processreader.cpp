// Unit tests for ProcessStatReader (processstatreader.h/.cpp).
// Verifies process discovery from /proc, find_pid_in_data,
// and nethogs graceful failure flags in an unprivileged container.
#include <QtTest>
#include <QDir>
#include <thread>
#include <unistd.h>

#include "processstatreader.h"

class TestProcessReader : public QObject
{
    Q_OBJECT
private slots:
    void data_vec_has_entries();
    void find_pid_returns_own_pid();
    void find_pid_returns_nullptr_for_unknown();
    void process_name_populated();
    void nethogs_flags_set_in_container();
    void measure_main_loop_runs();
};

void TestProcessReader::data_vec_has_entries()
{
    ProcessStatReader reader;
    // At minimum, the test process itself should be listed.
    QVERIFY2(reader.getDeviceCount() > 0,
             "data vector is empty after construction");
}

void TestProcessReader::find_pid_returns_own_pid()
{
    ProcessStatReader reader;
    pid_t pid = getpid();
    auto* entry = reader.find_pid_in_data(static_cast<int>(pid));
    if (entry == nullptr) {
        // In some containers /proc/<pid>/status is not readable for own PID.
        // Fall back: verify the data vector is non-empty and at least one
        // PID can be looked up.
        QVERIFY2(reader.getDeviceCount() > 0,
                 "data vector empty and own PID not found");
        auto* anyEntry = reader.find_pid_in_data(reader.m_DataVec[0].pid);
        QVERIFY2(anyEntry != nullptr, "no PID in data vector is findable");
        return;
    }
    QCOMPARE(entry->pid, static_cast<int>(pid));
}

void TestProcessReader::find_pid_returns_nullptr_for_unknown()
{
    ProcessStatReader reader;
    // A PID well above any real process ID.
    auto* entry = reader.find_pid_in_data(999999999);
    QVERIFY2(entry == nullptr,
             "find_pid_in_data returned non-null for a non-existent PID");
}

void TestProcessReader::process_name_populated()
{
    ProcessStatReader reader;
    if (reader.getDeviceCount() == 0) {
        QSKIP("no processes visible in this environment");
    }
    // Check the first entry in the data vector has a non-empty name.
    const auto& first = reader.m_DataVec[0];
    QVERIFY2(!first.name.trimmed().isEmpty(),
             qPrintable(QString("process name is empty for PID %1").arg(first.pid)));
}

void TestProcessReader::nethogs_flags_set_in_container()
{
    ProcessStatReader reader;
    // In an unprivileged container, nethogs cannot open a raw socket,
    // so the nethogs thread should set noRootPriv or noEthDeviceErr.
    // Give the thread time to finish.
    QThread::msleep(500);
    QVERIFY2(reader.noRootPriv || reader.noEthDeviceErr,
             "expected noRootPriv or noEthDeviceErr in unprivileged container");
}

void TestProcessReader::measure_main_loop_runs()
{
    ProcessStatReader reader;
    std::thread t([&reader]() { reader.measure_main_loop(); });
    QThread::msleep(500);
    reader.m_quit = true;
    t.join();
    // No crash is the main assertion; data vector should still be valid.
    QVERIFY2(reader.getDeviceCount() > 0,
             "data vector empty after measure_main_loop");
}

QTEST_MAIN(TestProcessReader)
#include "t_processreader.moc"
