// Unit tests for the CPU stats module (cpustats.h/.cpp).
// Covers /proc/stat parsing, cpu counting, activity calculation and the
// idle/active time helpers.
#include <QtTest>
#include <vector>
#include <string>

// Test-only: access private members of the legacy class under test.
#define private public
#include "cpustats.h"
#undef private

class TestCPUStats : public QObject
{
    Q_OBJECT
private slots:
    void cpu_count_positive();
    void cpu_count_matches_proc_stat();
    void activity_vector_size();
    void activity_values_in_range();
    void activity_pairs_consistent();
    void idle_plus_active_covers_time();
    void get_idle_time();
    void get_active_time();

private:
    // Count "cpuN" lines in /proc/stat directly (independent of the class under test).
    static int count_proc_stat_cpu_lines();
};

int TestCPUStats::count_proc_stat_cpu_lines()
{
    std::ifstream f("/proc/stat");
    int count = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.compare(0, 3, "cpu") == 0)
            ++count;
    }
    return count;
}

void TestCPUStats::cpu_count_positive()
{
    QCOMPARE(CPUStats::get_cpu_count() > 0, true);
}

void TestCPUStats::cpu_count_matches_proc_stat()
{
    // /proc/stat contains 1 aggregate "cpu" line + one line per core.
    // get_cpu_count() returns total lines - 1, i.e. the number of cores.
    QCOMPARE(static_cast<int>(CPUStats::get_cpu_count()), count_proc_stat_cpu_lines() - 1);
}

void TestCPUStats::activity_vector_size()
{
    // First call seeds previous entries; second call returns one value per cpu line
    // (aggregate + N cores = N+1 entries).
    (void)CPUStats::get_cpus_activity_vector();
    auto vec = CPUStats::get_cpus_activity_vector();
    QCOMPARE(static_cast<int>(vec.size()), count_proc_stat_cpu_lines());
}

void TestCPUStats::activity_values_in_range()
{
    (void)CPUStats::get_cpus_activity_vector();
    QThread::msleep(150); // ensure non-zero total time between samples
    auto vec = CPUStats::get_cpus_activity_vector();
    for (float v : vec) {
        QVERIFY2(std::isfinite(v), qPrintable(QString("activity not finite: %1").arg(v)));
        QVERIFY2(v >= 0.0f && v <= 100.0f,
                 qPrintable(QString("activity value out of range: %1").arg(v)));
    }
}

void TestCPUStats::activity_pairs_consistent()
{
    // Let two measurements settle.
    (void)CPUStats::get_cpus_activity_vector();
    QThread::msleep(150);
    auto pairs = CPUStats::get_cpus_activity();

    // First entry is "tot" (aggregate), then cores are "0","1",...
    QVERIFY2(!pairs.empty(), "expected non-empty activity pairs");
    QCOMPARE(pairs.front().first, std::string("tot"));
    for (size_t i = 1; i < pairs.size(); ++i) {
        QCOMPARE(pairs[i].first, std::to_string(i - 1));
        QVERIFY(std::isfinite(pairs[i].second));
        QVERIFY(pairs[i].second >= 0.0f && pairs[i].second <= 100.0f);
    }
}

void TestCPUStats::idle_plus_active_covers_time()
{
    (void)CPUStats::get_cpus_activity_vector();
    // Back-to-back call with NO sleep: previously produced NaN when
    // TOTAL_TIME == 0.  The guard in get_cpus_activity_vector() now
    // returns 0.0f for zero-elapsed-time entries, so all values must
    // be finite.
    auto vec = CPUStats::get_cpus_activity_vector();
    for (float v : vec) {
        QVERIFY2(std::isfinite(v),
                 qPrintable(QString("activity not finite on zero-elapsed sample: %1").arg(v)));
    }
}

void TestCPUStats::get_idle_time()
{
    CPUStats::CPUData e{};
    e.times[CPUStats::S_IDLE] = 100;
    e.times[CPUStats::S_IOWAIT] = 25;
    for (int i = 0; i < CPUStats::NUM_CPU_STATES; ++i) e.times[i] = 0;
    e.times[CPUStats::S_IDLE] = 100;
    e.times[CPUStats::S_IOWAIT] = 25;
    QCOMPARE(CPUStats::GetIdleTime(e), static_cast<size_t>(125));
}

void TestCPUStats::get_active_time()
{
    CPUStats::CPUData e{};
    for (int i = 0; i < CPUStats::NUM_CPU_STATES; ++i) e.times[i] = 10;
    // Active = everything except IOWAIT and IDLE: 8 * 10 = 80.
    QCOMPARE(CPUStats::GetActiveTime(e), static_cast<size_t>(80));
}

QTEST_MAIN(TestCPUStats)
#include "t_cpustats.moc"
