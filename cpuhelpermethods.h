#ifndef CPUHELPERMETHODS_H
#define CPUHELPERMETHODS_H

#include <string>


class CPUHelperMethods
{
public:
    struct LCache{
        int L1CacheSize=0;
        int L2CacheSize=0;
        int L3CacheSize=0;
    };

    static std::string getCPUName();                //returns name of cpu as std::string
    static std::string getCPUCount();               //returns number of physical cores of cpu as std::string
    static std::string getCPUBaseClockSpeed();      //returns number of logical cores of cpu as std::string
    static LCache cacheSize();                      //returns the CPU cache sizes

private:
    CPUHelperMethods();

};

#endif // CPUHELPERMETHODS_H
