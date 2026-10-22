#ifndef PROCESSGROUP_H
#define PROCESSGROUP_H

#include <QString>
#include <QVector>
#include "processstatreader.h"

namespace StatTypes{

// One aggregated row for all processes sharing the same name (issue #12).
struct ProcessGroup{
    QString name;                  //!< process name the group is keyed by (trimmed, lowercase)
    QString displayName = "";      //!< original name as shown in /proc (first seen)
    QString user = "";             //!< user of the first process in the group
    double totalCpuPct = 0.0;      //!< summed CPU usage of all member processes
    double totalMemKiB = 0.0;      //!< summed memory of all member processes
    double totalDiskKiBs = 0.0;    //!< summed disk read+write rate
    double totalNetKiBs = 0.0;     //!< summed network read+write rate
    QVector<int> pids;             //!< pids of all member processes
};

}

// Collapse a per-PID process list into groups keyed by process name.
// Summed values (CPU, memory, disk, net) and a pid list per group.
QVector<StatTypes::ProcessGroup> aggregate_processes(
    const QVector<StatTypes::ProcessData>& processList);

#endif // PROCESSGROUP_H
