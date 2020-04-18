#include "diskstatreader.h"

diskStatReader::diskData &diskStatReader::getDiskInfo(int diskIndex)
{

}

int diskStatReader::getDiskCount()
{

}

void diskStatReader::measure_main_loop()
{
    //dont forget the activity / transfer rate vectors should keeps cosntant size...so if you add in the back you have to pop in the front

    //please use following data filtering algothyimten for filtering the datavectors (smoothes always the last elemetns added)
    //filtering 2nd order to power n
    //int k = data.size()-3;
   // for(int n=8; n>=0; n--){
   // data[i]= (data[k-2] + data[k-1] + data[k] +data[k+1] +data[k+2] )/ 5.0;
   // k--;
   // }

}

void diskStatReader::start()
{

}

void diskStatReader::stop()
{

}

diskStatReader::diskStatReader()
{

}
