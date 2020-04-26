#ifndef CPUSTATS_H
#define CPUSTATS_H
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class CPUStats
{
public:
    static std::vector<std::pair<std::string,float>> get_cpus_activity();
    static std::vector<float> get_cpus_activity_vector();                       ///<jsut returns the activiy wihtout nametags, 0 elemten tis always the whole CPU stat
    static size_t get_cpu_count();
private:
    CPUStats();
static constexpr int NUM_CPU_STATES = 10;

enum CPUStates
{
    S_USER = 0,
    S_NICE,
    S_SYSTEM,
    S_IDLE,
    S_IOWAIT,
    S_IRQ,
    S_SOFTIRQ,
    S_STEAL,
    S_GUEST,
    S_GUEST_NICE
};

struct CPUData
{
    std::string cpu;
    size_t times[NUM_CPU_STATES];
};


static void ReadStatsCPU(std::vector<CPUData> & entries);
static size_t GetIdleTime(const CPUData & e);
static size_t GetActiveTime(const CPUData & e);



static std::vector<CPUData> m_CurrentEntries;
static std::vector<CPUData> m_PreviousEntries;



};

#endif // CPUSTATS_H
