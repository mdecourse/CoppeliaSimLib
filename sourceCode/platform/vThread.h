#pragma once

#include "vMutex.h"
#ifdef SIM_WITH_QT
    #include <QTime>
    #include <QThread>
#endif
#ifndef WIN_SIM
    #include <unistd.h>
    #include <pthread.h>
#endif

// FULLY STATIC CLASS
class VThread  
{
public:
    static void launchThread(VTHREAD_START_ADDRESS startAddress,bool followMainThreadAffinity);
    static void endThread();
#ifdef SIM_WITH_QT
    static void launchSimpleThread(SIMPLE_VTHREAD_START_ADDRESS startAddress);
#endif
    static void endSimpleThread();
    static void setProcessorCoreAffinity(int mode); // negative=multicore, 0=any single core, otherwise affinity mask (bit1=core1, bit2=core2, etc.)
    static int getThreadId_apiQueried();
    static int getCoreCount();
    static bool isSimulationMainThreadIdSet();
    static void setSimulationMainThreadId();
    static void unsetSimulationMainThreadId();
    static bool isUiThreadIdSet();
    static void setUiThreadId();
    static void unsetUiThreadId();
    static bool isCurrentThreadTheMainSimulationThread();
    static bool isCurrentThreadTheUiThread();
    static bool isCurrentThreadNotTheUiThreadOrUiThreadNotYetSet();
    static bool areThreadIDsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB);
    static VTHREAD_ID_TYPE getCurrentThreadId();
    static void switchThread();
    static void sleep(int ms);

private:
    static bool _simulationMainThreadIdSet;
    static bool _uiThreadIdSet;
    static VTHREAD_ID_TYPE _simulationMainThreadId;
    static VTHREAD_ID_TYPE _uiThreadId;
    static VMutex _lock;
    static std::vector<VTHREAD_ID_TYPE> _apiQueriedThreadIds;
    static VTHREAD_AFFINITY_MASK _mainThreadAffinityMask;
};

