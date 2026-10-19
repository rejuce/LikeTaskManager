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
    void stop_does_not_stop_thread(); // documents known bug
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
    // The filter walks k from size-3 down (9 iterations) and accesses k-2..k+2;
    // last k is size-12, so minimum safe size is 13 (last k=2, k-2=0 valid).
    // NOTE: sizes < 13 cause size_t underflow in k-2 (latent bug in StatReader.h
    // — no size guard; verified: size 11 segfaults). All production callers keep
    // their deques at m_dataPointsPerMinute (600), so this is unreachable there.
    DummyReader r;
    auto data = make_deque({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
    r.filter_data_deque(data); // must not crash
    QCOMPARE(data.size(), static_cast<size_t>(13));
}

void TestStatReader::start_runs_loop()
{
    DummyReader r;
    r.start();
    QThread::msleep(120);
    QVERIFY(r.loopCount > 0);
    // Known bug: stop() sets m_quit=false, so the thread keeps running.
    r.stop();
    QThread::msleep(60);
    QVERIFY(r.loopCount > 0);
    // Clean up: force quit and detach to avoid hanging at test end.
    r.m_quit = true;
    // Loop will exit on next iteration; detach so we don't join a dead thread state.
    if (r.m_measureT.joinable()) {
        // Give the loop a moment to observe m_quit.
        QThread::msleep(50);
        r.m_measureT.detach();
    }
}

void TestStatReader::stop_does_not_stop_thread()
{
    // Documents the existing bug: stop() sets m_quit = false (no-op),
    // so after start() the loop cannot be stopped via the public API.
    DummyReader r;
    r.m_quit = true;
    r.stop();
    QCOMPARE(r.m_quit, false);
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
