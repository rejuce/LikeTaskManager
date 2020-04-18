#include "cpustats.h"
 std::vector<CPUStats::CPUData> CPUStats::m_CurrentEntries;
 std::vector<CPUStats::CPUData> CPUStats::m_PreviousEntries;

std::vector<std::pair<std::string, float> > CPUStats::get_cpus_activity()
{
    std::vector<std::pair<std::string, float> > retvec;

    ReadStatsCPU(m_CurrentEntries);


    if(m_PreviousEntries.size()>0){




        for(size_t i = 0; i < m_CurrentEntries.size(); ++i)
        {
            const CPUData & e1 = m_PreviousEntries[i];
            const CPUData & e2 = m_CurrentEntries[i];



            const float ACTIVE_TIME	= static_cast<float>(GetActiveTime(e2) - GetActiveTime(e1));
            const float IDLE_TIME	= static_cast<float>(GetIdleTime(e2) - GetIdleTime(e1));
            const float TOTAL_TIME	= ACTIVE_TIME + IDLE_TIME;

            retvec.emplace_back(std::pair<std::string, float>(m_CurrentEntries[i].cpu,100.f * ACTIVE_TIME / TOTAL_TIME));


        }
    }

    m_PreviousEntries=m_CurrentEntries;
    return retvec;


}

CPUStats::CPUStats()
{




}
void CPUStats::ReadStatsCPU(std::vector<CPUData> & entries)
{
    entries.clear();

    std::ifstream fileStat("/proc/stat");

    std::string line;

    const std::string STR_CPU("cpu");
    const std::size_t LEN_STR_CPU = STR_CPU.size();
    const std::string STR_TOT("tot");

    while(std::getline(fileStat, line))
    {
        // cpu stats line found
        if(!line.compare(0, LEN_STR_CPU, STR_CPU))
        {
            std::istringstream ss(line);

            // store entry
            entries.emplace_back(CPUData());
            CPUData & entry = entries.back();

            // read cpu label
            ss >> entry.cpu;

            // remove "cpu" from the label when it's a processor number
            if(entry.cpu.size() > LEN_STR_CPU)
                entry.cpu.erase(0, LEN_STR_CPU);
            // replace "cpu" with "tot" when it's total values
            else
                entry.cpu = STR_TOT;

            // read times
            for(int i = 0; i < NUM_CPU_STATES; ++i)
                ss >> entry.times[i];
        }
    }
}

size_t CPUStats::GetIdleTime(const CPUData & e)
{
    return	e.times[S_IDLE] +
            e.times[S_IOWAIT];
}

size_t CPUStats::GetActiveTime(const CPUData & e)
{
    return	e.times[S_USER] +
            e.times[S_NICE] +
            e.times[S_SYSTEM] +
            e.times[S_IRQ] +
            e.times[S_SOFTIRQ] +
            e.times[S_STEAL] +
            e.times[S_GUEST] +
            e.times[S_GUEST_NICE];
}

size_t CPUStats::get_cpu_count()
{
    ReadStatsCPU(m_CurrentEntries);
    return m_CurrentEntries.size()-1;
}
