#include "processgroup.h"
#include <QHash>

QVector<StatTypes::ProcessGroup> aggregate_processes(
    const QVector<StatTypes::ProcessData>& processList)
{
    // First-occurrence order, so group rows stay stable between refreshes.
    QHash<QString, int> nameToIndex;
    QVector<StatTypes::ProcessGroup> groups;

    for(const auto& proc : processList){
        const QString key = proc.name.trimmed().toLower();
        if(key.isEmpty()) continue;

        int index;
        auto it = nameToIndex.find(key);
        if(it == nameToIndex.end()){
            index = groups.size();
            StatTypes::ProcessGroup group;
            group.name = key;
            group.displayName = proc.name.trimmed();
            group.user = proc.user;
            groups.push_back(std::move(group));
            nameToIndex.insert(key, index);
        } else {
            index = it.value();
        }

        auto& group = groups[index];
        group.totalCpuPct += proc.currentCPUPct;
        group.totalMemKiB += proc.currentMemKiB;
        group.totalDiskKiBs += proc.currentReadKiBsDiskData + proc.currentWriteKiBsDiskData;
        group.totalNetKiBs += proc.currentReadKiBsNetData + proc.currentWriteKiBsNetData;
        group.pids.push_back(proc.pid);
    }

    return groups;
}
