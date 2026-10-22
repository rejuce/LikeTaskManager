// Unit tests for aggregate_processes() (processgroup.h/.cpp) — issue #12.
// Verifies that per-PID process lists collapse into name-keyed groups
// with summed CPU/memory/disk/net and a complete pid list.
#include <QtTest>
#include <QVector>
#include "processgroup.h"

static StatTypes::ProcessData make_proc(int pid, const QString& name, const QString& user,
                                        double cpuPct, double memKiB,
                                        double rdKiBs, double wrKiBs,
                                        double netRdKiBs, double netWrKiBs)
{
    StatTypes::ProcessData p;
    p.pid = pid;
    p.name = name;
    p.user = user;
    p.currentCPUPct = cpuPct;
    p.currentMemKiB = memKiB;
    p.currentReadKiBsDiskData = rdKiBs;
    p.currentWriteKiBsDiskData = wrKiBs;
    p.currentReadKiBsNetData = netRdKiBs;
    p.currentWriteKiBsNetData = netWrKiBs;
    return p;
}

class TestProcessGroup : public QObject
{
    Q_OBJECT
private slots:
    void empty_list_gives_no_groups();
    void single_process_gives_one_group();
    void same_name_processes_are_grouped();
    void different_names_stay_separate();
    void grouping_is_case_insensitive();
    void empty_name_is_skipped();
};

void TestProcessGroup::empty_list_gives_no_groups()
{
    auto groups = aggregate_processes({});
    QCOMPARE(groups.size(), 0);
}

void TestProcessGroup::single_process_gives_one_group()
{
    auto groups = aggregate_processes({
        make_proc(1, "bash", "root", 0.05, 1024, 10, 20, 1, 2)
    });
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].name, QString("bash"));
    QCOMPARE(groups[0].displayName, QString("bash"));
    QCOMPARE(groups[0].user, QString("root"));
    QCOMPARE(groups[0].pids.size(), 1);
    QCOMPARE(groups[0].pids[0], 1);
    QCOMPARE(groups[0].totalCpuPct, 0.05);
    QCOMPARE(groups[0].totalMemKiB, 1024.0);
    QCOMPARE(groups[0].totalDiskKiBs, 30.0);
    QCOMPARE(groups[0].totalNetKiBs, 3.0);
}

void TestProcessGroup::same_name_processes_are_grouped()
{
    auto groups = aggregate_processes({
        make_proc(10, "chrome", "alice", 0.10, 1000,  5,  5, 1, 1),
        make_proc(11, "chrome", "alice", 0.20, 2000, 10, 10, 2, 2),
        make_proc(12, "chrome", "alice", 0.30, 3000, 15, 15, 3, 3)
    });
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].name, QString("chrome"));
    QCOMPARE(groups[0].pids.size(), 3);
    QCOMPARE(groups[0].pids, (QVector<int>{10, 11, 12}));
    QCOMPARE(groups[0].totalCpuPct, 0.60);
    QCOMPARE(groups[0].totalMemKiB, 6000.0);
    QCOMPARE(groups[0].totalDiskKiBs, 60.0);
    QCOMPARE(groups[0].totalNetKiBs, 12.0);
}

void TestProcessGroup::different_names_stay_separate()
{
    auto groups = aggregate_processes({
        make_proc(1, "bash",   "root", 0.1, 100, 0, 0, 0, 0),
        make_proc(2, "python", "root", 0.2, 200, 0, 0, 0, 0),
        make_proc(3, "bash",   "root", 0.3, 300, 0, 0, 0, 0)
    });
    QCOMPARE(groups.size(), 2);
    QCOMPARE(groups[0].name, QString("bash"));
    QCOMPARE(groups[0].pids.size(), 2);
    QCOMPARE(groups[0].totalCpuPct, 0.4);
    QCOMPARE(groups[1].name, QString("python"));
    QCOMPARE(groups[1].pids.size(), 1);
}

void TestProcessGroup::grouping_is_case_insensitive()
{
    auto groups = aggregate_processes({
        make_proc(1, "Xorg",  "root", 0.1, 100, 0, 0, 0, 0),
        make_proc(2, "xorg",  "root", 0.2, 200, 0, 0, 0, 0),
        make_proc(3, "XORG",  "root", 0.3, 300, 0, 0, 0, 0)
    });
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].name, QString("xorg"));
    QCOMPARE(groups[0].displayName, QString("Xorg")); // first-seen display name
    QCOMPARE(groups[0].pids.size(), 3);
}

void TestProcessGroup::empty_name_is_skipped()
{
    auto groups = aggregate_processes({
        make_proc(1, "  ", "root", 0.1, 100, 0, 0, 0, 0),
        make_proc(2, "bash", "root", 0.1, 100, 0, 0, 0, 0)
    });
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].name, QString("bash"));
}

QTEST_MAIN(TestProcessGroup)
#include "t_processgroup.moc"
