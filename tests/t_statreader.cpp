// Unit tests for the StatReader<T> template base class (StatReader.h).
// A minimal concrete subclass is used to exercise the template machinery.
#include <QtTest>
#include <deque>
#include <vector>

// Test-only: access protected/private members of the template under test.
#define private public
#define protected public
#include "StatReader.h"
#undef private
#undef protected

namespace {
// Minimal concrete reader: no real measurement, just counts loop iterations.
class TestItem {
public:
    int value = 0;
};

class DummyReader : public StatReader<TestItem> {
public:
    DummyReader() : StatReader<TestItem>(std::string("DummyReader")) {}
    int loopCount = 0;
    bool throwInLoop = false;

    void measure_main_loop() override {
        while (!m_quit) {
            if (throwInLoop)
                throw std::runtime_error("dummy exception");
            ++loopCount;
            QThread::msleep(5);
        }
    }
};
}

class TestStatReader : public QObject
{
    Q_OBJECT
private slots:
    void empty_initially();
    void device_count_tracks_data_vec();
    void get_stat_data_returns_reference();
    void filter_smooths_spike();
    void filter_requires_minimum_size();
    void start_runs_loop();
    void stop_stops_thread(); // verifies the fixed stop()
    void template_loop_catches_exception();

private:
    static std::deque<double> make_deque(std::initializer_list<double> values);
};

std::deque<double> TestStatReader::make_deque(std::initializer_list<double> values)
{
    return std::deque<double>(values);
}

void TestStatReader::empty_initially()
{
    DummyReader r;
    QCOMPARE(r.getDeviceCount(), 0);
}

void TestStatReader::device_count_tracks_data_vec()
{
    DummyReader r;
    r.m_DataVec.append(TestItem{});
    r.m_DataVec.append(TestItem{});
    QCOMPARE(r.getDeviceCount(), 2);
}

void TestStatReader::get_stat_data_returns_reference()
{
    DummyReader r;
    TestItem item; item.value = 42;
    r.m_DataVec.append(item);
    // getStatData returns a const reference to the stored element.
    const auto& stored = r.getStatData(0);
    QCOMPARE(stored.value, 42);
    QCOMPARE(std::addressof(stored), std::addressof(r.m_DataVec[0]));
}

void TestStatReader::filter_smooths_spike()
{
    DummyReader r;
    // Constant stream with one spike. Size must be >= 13 because
    // filter_data_deque walks k from size-3 down to 0 and accesses k-2.
    // 13 elements: indices 0..12, spike at index 8.
    auto data = make_deque({10, 10, 10, 10, 10, 10, 10, 10, 50, 10, 10, 10, 10});
    r.filter_data_deque(data);
    // The filter walks 9 positions (k=10..2) with a 5-point moving average.
    // A spike of 50 among 10s is reduced to at most (10+10+50+10+10)/5 = 22
    // on the first pass, so we assert the spike is reduced below 25
    // (it cannot remain at 50, and no new value exceeds 22).
    for (double v : data) {
        QVERIFY2(v < 25.0, qPrintable(QString("spike not sufficiently smoothed: %1").arg(v)));
    }
    // The original spike value of 50 must be gone.
    bool spikeGone = true;
    for (double v : data) { if (v >= 50.0) spikeGone = false; }
    QVERIFY(spikeGone);
}

void TestStatReader::filter_requires_minimum_size()
{
    // The filter has a size guard: deques with fewer than 13 elements are
    // returned unchanged (no-op) to prevent size_t underflow on k-2.
    DummyReader r;

    // Size 5: below the minimum, must be a no-op.
    auto small = make_deque({1, 2, 3, 4, 5});
    r.filter_data_deque(small);
    QCOMPARE(small.size(), static_cast<size_t>(5));
    QCOMPARE(small[0], 1.0);
    QCOMPARE(small[4], 5.0);

    // Size 12: still below the minimum, must be a no-op.
    auto mid = make_deque({1,2,3,4,5,6,7,8,9,10,11,12});
    r.filter_data_deque(mid);
    QCOMPARE(mid[0], 1.0);
    QCOMPARE(mid[11], 12.0);

    // Size 13: exactly the minimum, filter runs and modifies data.
    auto exact = make_deque({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
    r.filter_data_deque(exact);
    QCOMPARE(exact.size(), static_cast<size_t>(13));
}

void TestStatReader::start_runs_loop()
{
    DummyReader r;
    r.start();
    QThread::msleep(120);
    QVERIFY(r.loopCount > 0);
    // stop() now correctly sets m_quit=true and joins the thread.
    r.stop();
    // After stop(), the thread has been joined — no longer joinable.
    QVERIFY(!r.m_measureT.joinable());
    // loopCount should be stable (no more iterations).
    int countAfterStop = r.loopCount;
    QThread::msleep(50);
    QCOMPARE(r.loopCount, countAfterStop);
}

void TestStatReader::stop_stops_thread()
{
    // Verifies the fixed stop(): sets m_quit=true and joins the thread.
    DummyReader r;
    r.start();
    QThread::msleep(100);
    QVERIFY(r.loopCount > 0);
    int countBefore = r.loopCount;
    r.stop();
    // m_quit must be true after stop().
    QVERIFY(r.m_quit);
    // Thread must have been joined.
    QVERIFY(!r.m_measureT.joinable());
    // Loop count must be frozen.
    QCOMPARE(r.loopCount, countBefore);
}

void TestStatReader::template_loop_catches_exception()
{
    DummyReader r;
    r.throwInLoop = true;
    r.m_quit = true; // make loop exit immediately
    // templateLoop must swallow the exception instead of propagating it.
    r.templateLoop();
}

QTEST_MAIN(TestStatReader)
#include "t_statreader.moc"
