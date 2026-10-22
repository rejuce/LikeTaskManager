// Unit tests for the item widgets: PerfItemWidget (base),
// CPUItemWidget, RamItemWidget, DiskItemWidget, NetworkItemWidget.
// Runs offscreen; verifies label text updates and plot-type enums.
#include <QtTest>
#include <QListWidget>
#include <QApplication>

#include "cpuitemwidget.h"
#include "ramitemwidget.h"
#include "diskitemwidget.h"
#include "networkitemwidget.h"
#include "perfitemwidget.h"

class TestWidgets : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    // --- PerfItemWidget base ---
    void perf_item_plot_type_stored();
    void perf_item_labels_created();

    // --- CPUItemWidget ---
    void cpu_item_plot_type();
    void cpu_item_update_data();
    void cpu_item_update_data_zero();

    // --- RamItemWidget ---
    void ram_item_plot_type();
    void ram_item_update_data();
    void ram_item_update_data_zero_total_no_crash();

    // --- DiskItemWidget ---
    void disk_item_plot_type();
    void disk_item_name_in_first_line();
    void disk_item_update_data();

    // --- NetworkItemWidget ---
    void net_item_plot_type();
    void net_item_name_in_first_line();
    void net_item_update_data_kbs_range();
    void net_item_update_data_mbs_range();

private:
    QListWidget* m_view = nullptr;
};

void TestWidgets::initTestCase()
{
    m_view = new QListWidget;
    m_view->show(); // needed offscreen so setItemWidget works
}

void TestWidgets::cleanupTestCase()
{
    if (m_view) {
        delete m_view;
        m_view = nullptr;
    }
}

// --- PerfItemWidget base ---

void TestWidgets::perf_item_plot_type_stored()
{
    CPUItemWidget w("CPU Test", m_view);
    QCOMPARE(w.getPlotType(), PlotType::CPU);

    RamItemWidget r("RAM Test", m_view);
    QCOMPARE(r.getPlotType(), PlotType::RAM);

    DiskItemWidget d("Disk", "sda", m_view);
    QCOMPARE(d.getPlotType(), PlotType::DISK);

    NetworkItemWidget n("eth0", m_view);
    QCOMPARE(n.getPlotType(), PlotType::Ethernet);
}

void TestWidgets::perf_item_labels_created()
{
    // Two-line widgets (isThreeLined=false) → no secondLineTxt
    CPUItemWidget cpu("CPU", m_view);
    QVERIFY(cpu.m_firstLineTxt != nullptr);
    QVERIFY(cpu.m_secondLineTxt == nullptr);

    // Three-line widgets (isThreeLined=true) → both labels
    DiskItemWidget disk("Disk", "sda", m_view);
    QVERIFY(disk.m_firstLineTxt != nullptr);
    QVERIFY(disk.m_secondLineTxt != nullptr);

    NetworkItemWidget net("eth0", m_view);
    QVERIFY(net.m_firstLineTxt != nullptr);
    QVERIFY(net.m_secondLineTxt != nullptr);
}

// --- CPUItemWidget ---

void TestWidgets::cpu_item_plot_type()
{
    CPUItemWidget w("CPU0", m_view);
    QCOMPARE(w.getPlotType(), PlotType::CPU);
}

void TestWidgets::cpu_item_update_data()
{
    CPUItemWidget w("CPU0", m_view);
    w.update_data(42, 3.50);
    QCOMPARE(w.m_firstLineTxt->text(),
             QString("42%  3.50 Ghz"));
}

void TestWidgets::cpu_item_update_data_zero()
{
    CPUItemWidget w("CPU0", m_view);
    w.update_data(0, 0.0);
    QCOMPARE(w.m_firstLineTxt->text(),
             QString("0%  0.00 Ghz"));
}

// --- RamItemWidget ---

void TestWidgets::ram_item_plot_type()
{
    RamItemWidget w("RAM", m_view);
    QCOMPARE(w.getPlotType(), PlotType::RAM);
}

void TestWidgets::ram_item_update_data()
{
    RamItemWidget w("RAM", m_view);
    w.update_data(8.5, 16.0);
    // Expected: "8.5/16.0 GiB (53.1%)"
    QCOMPARE(w.m_firstLineTxt->text(),
             QString("8.5/16.0 GiB (53.1%)"));
}

void TestWidgets::ram_item_update_data_zero_total_no_crash()
{
    RamItemWidget w("RAM", m_view);
    w.update_data(1.0, 0.0); // total == 0 → no text update (guard)
    // Text should remain the initial " " (space)
    QVERIFY(w.m_firstLineTxt->text().trimmed().isEmpty());
}

// --- DiskItemWidget ---

void TestWidgets::disk_item_plot_type()
{
    DiskItemWidget w("Disk", "sda", m_view);
    QCOMPARE(w.getPlotType(), PlotType::DISK);
}

void TestWidgets::disk_item_name_in_first_line()
{
    DiskItemWidget w("Disk", "nvme0n1", m_view);
    QCOMPARE(w.m_firstLineTxt->text(), QString("nvme0n1"));
}

void TestWidgets::disk_item_update_data()
{
    DiskItemWidget w("Disk", "sda", m_view);
    QString read = "100 KB/s";
    QString write = "50 KB/s";
    QString active = "10";
    w.update_data(read, write, active);
    QCOMPARE(w.m_secondLineTxt->text(),
             QString("Rd.: 100 KB/s     Wd.: 50 KB/s (10%)"));
}

// --- NetworkItemWidget ---

void TestWidgets::net_item_plot_type()
{
    NetworkItemWidget w("eth0", m_view);
    QCOMPARE(w.getPlotType(), PlotType::Ethernet);
}

void TestWidgets::net_item_name_in_first_line()
{
    NetworkItemWidget w("wlan0", m_view);
    QCOMPARE(w.m_firstLineTxt->text(), QString("wlan0"));
}

void TestWidgets::net_item_update_data_kbs_range()
{
    NetworkItemWidget w("eth0", m_view);
    // Values < 2000 → KB/s format
    w.update_data(200, 500);  // send=200, rec=500
    // Rx: 500 KB/s, Tx: -200 KB/s
    QCOMPARE(w.m_secondLineTxt->text(),
             QString("Rx.: 500 KB/s   Tx.: -200 KB/s"));
}

void TestWidgets::net_item_update_data_mbs_range()
{
    NetworkItemWidget w("eth0", m_view);
    // Values >= 2000 → MB/s format
    w.update_data(2500, 3000);  // send=2500, rec=3000
    // Rx: 3000/1000=3 MB/s, Tx: -2500/1000=-3 MB/s (negative, <2000 → KB/s? No: -2500 < 2000 → KB/s)
    // Actually: currentTx = -2500, which is < 2000, so KB/s: "-2500 KB/s"
    // currentRx = 3000, which is >= 2000, so MB/s: "3 MB/s"
    QCOMPARE(w.m_secondLineTxt->text(),
             QString("Rx.: 3 MB/s   Tx.: -2500 KB/s"));
}

QTEST_MAIN(TestWidgets)
#include "t_widgets.moc"
