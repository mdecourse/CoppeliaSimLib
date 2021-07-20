#include "threadPool_old.h"
#include "app.h"
#include "vDateTime.h"
#include <boost/lexical_cast.hpp>

#ifdef MAC_SIM
    #include <curses.h>
#endif

VTHREAD_ID_TYPE CThreadPool_old::_threadToIntercept=0;
VTHREAD_START_ADDRESS CThreadPool_old::_threadInterceptCallback=nullptr;
int CThreadPool_old::_threadInterceptIndex=0;
int CThreadPool_old::_processorCoreAffinity=0;
int CThreadPool_old::_lockStage=0;
bool CThreadPool_old::_threadShouldNotSwitch_override=false;
std::vector<CVThreadData*> CThreadPool_old::_allThreadData;
std::vector<VTHREAD_ID_TYPE> CThreadPool_old::_threadQueue;
std::vector<int> CThreadPool_old::_threadStartTime;
bool CThreadPool_old::_simulationStopRequest=false;
int CThreadPool_old::_simulationStopRequestTime;
bool CThreadPool_old::_simulationEmergencyStopRequest=false;
VTHREAD_START_ADDRESS CThreadPool_old::_threadStartAdd=nullptr;
VMutex CThreadPool_old::_threadPoolMutex;

void* CThreadPool_old::_tmpData=nullptr;
int CThreadPool_old::_tmpRetData=0;
int CThreadPool_old::_inInterceptRoutine=0;

void CThreadPool_old::init()
{
    _threadToIntercept=0;
    _threadInterceptCallback=nullptr;
    _threadInterceptIndex=0;
    _processorCoreAffinity=0;
    _lockStage=0;
    _threadShouldNotSwitch_override=false;
    _allThreadData.clear();
    _threadQueue.clear();
    _threadStartTime.clear();
    _simulationStopRequest=false;
    _simulationEmergencyStopRequest=false;
    _threadStartAdd=nullptr;
    _tmpData=nullptr;
    _tmpRetData=0;
    _inInterceptRoutine=0;
}

VTHREAD_ID_TYPE CThreadPool_old::createNewThread(VTHREAD_START_ADDRESS threadStartAddress)
{
    _lock(0);

    if (_allThreadData.size()==0)
    { // We first need to add the main thread! executiontime: -1 --> not yet executed
        CVThreadData* dat=new CVThreadData(nullptr,VThread::getCurrentThreadId());
        _allThreadData.push_back(dat);

        _threadQueue.push_back(VThread::getCurrentThreadId());
        _threadStartTime.push_back(VDateTime::getTimeInMs());
        VThread::setProcessorCoreAffinity(_processorCoreAffinity);
    }

    // Following 2 new since 21/6/2014
    if (_allThreadData.size()==1)
        _threadShouldNotSwitch_override=false;

    _threadStartAdd=threadStartAddress;

    std::string tmp("==* Launching thread (from threadID: ");
    tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
    tmp+=")";
    App::logMsg(sim_verbosity_tracelua,tmp.c_str());

    VThread::launchThread(_intermediateThreadStartPoint,true);
    while (_threadStartAdd!=nullptr)
        VThread::sleep(1); // We wait until the thread could set its thread ID ************* TODO: don't use sleep!!!
    VTHREAD_ID_TYPE newID=(VTHREAD_ID_TYPE)_allThreadData[_allThreadData.size()-1]->threadID;

    tmp="==* Thread was created with ID: ";
    tmp+=boost::lexical_cast<std::string>((unsigned long)newID);
    tmp+=" (from thread ID: ";
    tmp+=boost::lexical_cast<std::string>((unsigned long)_threadQueue[_threadQueue.size()-1]);
    tmp+=")";
    App::logMsg(sim_verbosity_tracelua,tmp.c_str());
    _unlock(0);
    return(newID);
}

bool CThreadPool_old::setThreadSwitchTiming(int timeInMs)
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(1);
    bool retVal=false;
    int fqs=int(_threadQueue.size());
    if (fqs>1)
    {
        VTHREAD_ID_TYPE fID=_threadQueue[fqs-1];
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
            {
                if (timeInMs<0)
                    timeInMs=0;
                if (timeInMs>10000) // from 200 to 10000 on 13/3/2015
                    timeInMs=10000;
                _allThreadData[i]->threadDesiredTiming=timeInMs;
                retVal=true;
                break;
            }
        }
    }
    _unlock(1);
    return(retVal);
}

bool CThreadPool_old::getThreadSwitchTiming(int& timeInMs)
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(2);
    bool retVal=false;
    int fqs=int(_threadQueue.size());
    if (fqs>1)
    {
        VTHREAD_ID_TYPE fID=_threadQueue[fqs-1];
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
            {
                timeInMs=_allThreadData[i]->threadDesiredTiming;
                retVal=true;
                break;
            }
        }
    }
    _unlock(2);
    return(retVal);
}

bool CThreadPool_old::setThreadAutomaticSwitchForbidLevel(int forbidLevel)
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(1);
    bool retVal=false;
    int fqs=int(_threadQueue.size());
    if (fqs>1)
    {
        VTHREAD_ID_TYPE fID=_threadQueue[fqs-1];
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
            {
                _allThreadData[i]->threadShouldNotSwitch=forbidLevel;
                retVal=true;
                break;
            }
        }
    }
    _unlock(1);
    return(retVal);
}

bool CThreadPool_old::getThreadAutomaticSwitch()
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(1);
    bool retVal=true;
    int fqs=int(_threadQueue.size());
    if (fqs>1)
    {
        VTHREAD_ID_TYPE fID=_threadQueue[fqs-1];
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
            {
                retVal=(_allThreadData[i]->threadShouldNotSwitch==0);
                break;
            }
        }
    }
    _unlock(1);
    return(retVal);
}

bool CThreadPool_old::setThreadResumeLocation(int location,int order)
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(99);
    bool retVal=false;
    int fqs=int(_threadQueue.size());
    if (fqs>1)
    {
        VTHREAD_ID_TYPE fID=_threadQueue[fqs-1];
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
            {
                if (order<0)
                    order=-1;
                if (order>0)
                    order=1;
                if (int(_allThreadData[i]->threadResumeLocationAndOrder/3)!=location) // these 2 lines new since 4/6/2015 (forward-relocated scripts would only execute in next simulation pass)
                    _allThreadData[i]->allowToExecuteAgainInThisSimulationStep=true;
                _allThreadData[i]->threadResumeLocationAndOrder=location*3+(order+1);
                retVal=true;
                break;
            }
        }
    }
    _unlock(1);
    return(retVal);
}

void CThreadPool_old::switchToThread(VTHREAD_ID_TYPE threadID)
{
    if (_inInterceptRoutine>0)
        return;
    _lock(3);
    int fql=int(_threadQueue.size());
    if (!VThread::areThreadIDsSame(_threadQueue[fql-1],VThread::getCurrentThreadId()))
    {
        _unlock(3);
        return; // We have a free-running thread here (cannot be stopped if it doesn't itself disable the free-running mode)
    }
    for (size_t i=0;i<_allThreadData.size();i++)
    {
        if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,threadID))
        {
            VTHREAD_ID_TYPE oldFiberID=_threadQueue[fql-1];
            if ((fql>1)&&VThread::areThreadIDsSame(threadID,_threadQueue[fql-2]))
            { // we switch back to a previous (calling) fiber
                CVThreadData* it=nullptr;
                for (size_t j=0;j<_allThreadData.size();j++)
                {
                    if (VThread::areThreadIDsSame(_allThreadData[j]->threadID,oldFiberID))
                    {
                        it=_allThreadData[j];
                        break;
                    }
                }
                // Before we proceed, there is a special case to handle here:
                // When a thread was marked for termination, its fiberOrThreadID is 0xfffffff, and in
                // consequence, it would be nullptr here.

                if ( (it==nullptr)||(!it->threadSwitchShouldTriggerNoOtherThread) )
                { // Regular situation
                    _threadQueue.pop_back();
                    int totalTimeInMs=VDateTime::getTimeDiffInMs(_threadStartTime[fql-1]);
                    _threadStartTime.pop_back();
                    _threadStartTime[fql-2]+=totalTimeInMs; // We have to "remove" the time spent in the called fiber!

                    std::string tmp("==< Switching backward from threadID: ");
                    tmp+=boost::lexical_cast<std::string>((unsigned long)oldFiberID);
                    tmp+=" to threadID: ";
                    tmp+=boost::lexical_cast<std::string>((unsigned long)_allThreadData[i]->threadID);
                    App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                    _allThreadData[i]->threadWantsResumeFromYield=true; // We mark the next thread for resuming
                }
                else
                { // Happens when a thread that used to be free running (or that still is) comes through here
                    _threadQueue.pop_back();
                    _threadStartTime.pop_back();
                    if ( (it!=nullptr)&&(!it->threadShouldRunFreely) )
                    {
                        std::string tmp("==< Switching backward from previously free-running thread with ID: ");
                        tmp+=boost::lexical_cast<std::string>((unsigned long)oldFiberID);
                        App::logMsg(sim_verbosity_tracelua,tmp.c_str());
                    }
                }

                _unlock(3); // make sure to unlock here
                if (it!=nullptr)
                {
                    it->threadSwitchShouldTriggerNoOtherThread=false; // We have to reset this one
                    // Now we wait here until this thread gets flagged as threadWantsResumeFromYield:

                    std::string tmp("==< Backward switch part, threadID ");
                    tmp+=boost::lexical_cast<std::string>((unsigned long)it->threadID);
                    tmp+=" (";
                    tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
                    tmp+=") is waiting...";
                    App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                    while (!it->threadWantsResumeFromYield)
                    {
                        if (_threadToIntercept!=0)
                        {
                            if (VThread::areThreadIDsSame(_threadToIntercept,VThread::getCurrentThreadId()))
                            {
                                _threadToIntercept=0;
                                _threadInterceptCallback(nullptr);
                                _threadInterceptCallback=nullptr;
                                _threadInterceptIndex--;
                            }
                        }
                        VThread::switchThread();
                    }

                    tmp="==< Backward switch part, threadID ";
                    tmp+=boost::lexical_cast<std::string>((unsigned long)it->threadID);
                    tmp+=" (";
                    tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
                    tmp+=") NOT waiting anymore...";
                    App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                    // If we arrived here, it is because CThreadPool_old::switchToFiberOrThread was called for this thread from another thread
                    it->threadWantsResumeFromYield=false; // We reset it
                    // Now this thread resumes!
                }
                return;
            }
            else
            { // we switch forward to an auxiliary thread
                _threadQueue.push_back(threadID);
                _threadStartTime.push_back(VDateTime::getTimeInMs());

                std::string tmp("==> Switching forward from threadID: ");
                tmp+=boost::lexical_cast<std::string>((unsigned long)_threadQueue[_threadQueue.size()-2]);
                tmp+=" to threadID: ";
                tmp+=boost::lexical_cast<std::string>((unsigned long)threadID);
                App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                _allThreadData[i]->threadWantsResumeFromYield=true; // We mark the next thread for resuming
                // We do not need to idle this thread since it is already flagged as such
                CVThreadData* it=nullptr;
                for (size_t j=0;j<_allThreadData.size();j++)
                {
                    if (VThread::areThreadIDsSame(_allThreadData[j]->threadID,oldFiberID))
                    {
                        it=_allThreadData[j];
                        break;
                    }
                }
                // Now we wait here until this thread gets flagged as threadWantsResumeFromYield:
                _unlock(3);

                tmp="==> Forward switch part, threadID ";
                tmp+=boost::lexical_cast<std::string>((unsigned long)it->threadID);
                tmp+=" (";
                tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
                tmp+=") is waiting...";
                App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                while (!it->threadWantsResumeFromYield)
                    VThread::switchThread();

                tmp="==> Forward switch part, threadID ";
                tmp+=boost::lexical_cast<std::string>((unsigned long)it->threadID);
                tmp+=" (";
                tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
                tmp+=") NOT waiting anymore...";
                App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                // If we arrived here, it is because CThreadPool_old::switchToFiberOrThread was called for this thread from another thread
                it->threadWantsResumeFromYield=false; // We reset it
                // Now this thread resumes!
                _cleanupTerminatedThreads(); // This routine will perform the locking unlocking itself
                return;
            }
        }
    }
    _unlock(3);
}

bool CThreadPool_old::switchBackToPreviousThread()
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(4);
    int fql=int(_threadQueue.size());
    if (fql>1)
    { // Switch back only if not main thread, and not external thread
        if (!VThread::isCurrentThreadTheMainSimulationThread()) // new since 10/11/2014. Now thread switch can also be called from the C API
        {
            if (VThread::areThreadIDsSame(VThread::getCurrentThreadId(),_threadQueue[fql-1])) // new since 10/11/2014. Now thread switch can also be called from the C API
            {
                int totalTimeInMs=VDateTime::getTimeDiffInMs(_threadStartTime[fql-1]);
                for (size_t i=0;i<_allThreadData.size();i++)
                {
                    if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,_threadQueue[fql-1]))
                    {
                        _allThreadData[i]->threadExecutionTime=totalTimeInMs;
                        break;
                    }
                }
                _unlock(4);
                switchToThread(_threadQueue[fql-2]); // has its own locking / unlocking
                return(true);
            }
        }
    }
    _unlock(4);
    return(false);
}

bool CThreadPool_old::isSwitchBackToPreviousThreadNeeded()
{
    if (_inInterceptRoutine>0)
        return(false);
    _lock(5);
    int fql=int(_threadQueue.size());
    if (fql>1)
    { // Switch back only if not main thread
        int totalTimeInMs=VDateTime::getTimeDiffInMs(_threadStartTime[fql-1]);
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,_threadQueue[fql-1]))
            {
                if (_allThreadData[i]->threadDesiredTiming<=totalTimeInMs)
                {
                    if ((_allThreadData[i]->threadShouldNotSwitch==0)||_threadShouldNotSwitch_override)
                    {
                        _unlock(5);
                        return(true);
                    }
                }
                break;
            }
        }
    }
    _unlock(5);
    return(false);
}

void CThreadPool_old::switchBackToPreviousThreadIfNeeded()
{
    if (_inInterceptRoutine>0)
        return;
    _lock(5);
    int fql=int(_threadQueue.size());
    if (fql>1)
    { // Switch back only if not main thread
        int totalTimeInMs=VDateTime::getTimeDiffInMs(_threadStartTime[fql-1]);
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,_threadQueue[fql-1]))
            {
                if (_allThreadData[i]->threadDesiredTiming<=totalTimeInMs)
                {
                    if ((_allThreadData[i]->threadShouldNotSwitch==0)||_threadShouldNotSwitch_override)
                    {
                        _unlock(5);
                        switchBackToPreviousThread(); // Has its own locking / unlocking
                        return;
                    }
                }
                break;
            }
        }
    }
    _unlock(5);

    // Following new since 4/8/2014
    if (_threadShouldNotSwitch_override&&isThreadInFreeMode())
        setThreadFreeMode(false);
}

void CThreadPool_old::_terminateThread()
{
    _lock(6);
    VTHREAD_ID_TYPE nextThreadToSwitchTo=0;
    for (size_t i=0;i<_allThreadData.size();i++)
    {
        int fql=int(_threadQueue.size());
        if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,_threadQueue[fql-1]))
        {
            std::string tmp("==q Terminating thread: ");
            tmp+=boost::lexical_cast<std::string>((unsigned long)_allThreadData[i]->threadID);
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

            _allThreadData[i]->threadID=VTHREAD_ID_DEAD; // To indicate we need clean-up
            nextThreadToSwitchTo=_threadQueue[fql-2]; // This will be the next thread we wanna switch to
            break;
        }
    }
    _unlock(6);
    switchToThread(nextThreadToSwitchTo); // We switch to the calling thread (previous thread)
}

void CThreadPool_old::_cleanupTerminatedThreads()
{
    _lock(7);
    for (size_t i=0;i<_allThreadData.size();i++)
    {
        if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,VTHREAD_ID_DEAD))
        { // That thread died or is dying, we clean-up
            delete _allThreadData[i];
            _allThreadData.erase(_allThreadData.begin()+i);
            i--;
        }
    }
    _unlock(7);
}

VTHREAD_RETURN_TYPE CThreadPool_old::_intermediateThreadStartPoint(VTHREAD_ARGUMENT_TYPE lpData)
{
    srand(VDateTime::getTimeInMs()+ (((unsigned long)(VThread::getCurrentThreadId()))&0xffffffff) ); // Important: each thread starts with a same seed!!!
    VTHREAD_START_ADDRESS startAdd=_threadStartAdd;
    CVThreadData* it=new CVThreadData(nullptr,VThread::getCurrentThreadId());
    _allThreadData.push_back(it);
    _threadStartAdd=nullptr; // To indicate we could set the thread iD (in case of threads)
    it->threadWantsResumeFromYield=false;
    while (!it->threadWantsResumeFromYield)
        VThread::switchThread();

    // If we arrived here, it is because CThreadPool_old::switchToFiberOrThread was called for this thread from another thread
    it->threadWantsResumeFromYield=false; // We reset it

    std::string tmp("==* Inside new thread (threadID: ");
    tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
    tmp+=")";
    App::logMsg(sim_verbosity_tracelua,tmp.c_str());

    startAdd(lpData);

    // The thread ended

    _terminateThread();

    VThread::endThread();
    return(VTHREAD_RETURN_VAL);
}

void CThreadPool_old::prepareAllThreadsForResume_calledBeforeMainScript()
{
    _lock(8);
    for (size_t i=1;i<_allThreadData.size();i++)
    {
        // Following is a special condition to support free-running mode:
        if ( (!_allThreadData[i]->threadShouldRunFreely)&&(!_allThreadData[i]->threadSwitchShouldTriggerNoOtherThread) )
        {
            _allThreadData[i]->threadExecutionTime=-1; // -1 --> not yet executed
            _allThreadData[i]->allowToExecuteAgainInThisSimulationStep=false; // threads that are not relocated for resume can only run one time per simulation pass!
            _allThreadData[i]->usedMovementTime=0.0f; // we reset the used movement time at every simulation pass (better results than if we don't do it, tested!)
        }
    }
    _unlock(8);
}

int CThreadPool_old::handleAllThreads_withResumeLocation(int location)
{
    return(handleThread_ifHasResumeLocation(0,true,location));
}

int CThreadPool_old::handleThread_ifHasResumeLocation(VTHREAD_ID_TYPE theThread,bool allThreadsWithResumeLocation,int location)
{
    int retVal=0;
    _lock(8);
    bool doAll=false;
    if (location==-1) // Will resume all unhandled threads (to be called at the end of the main script)
    {
        location=0;
        doAll=true;
    }
    for (int j=3*location;j<3*location+3;j++)
    { // first, normal and last (execution order). Handled elsewhere now (but keep for the legacy functionality)
        for (size_t i=1;i<_allThreadData.size();i++)
        {
            if ( allThreadsWithResumeLocation||(_allThreadData[i]->threadID==theThread) )
            {
                if ((_allThreadData[i]->threadResumeLocationAndOrder==j)||doAll)
                { // We first execute those with 0, then 1, then 2! (then 3 in the sensing phase!)
                    if ( (_allThreadData[i]->threadExecutionTime==-1)||(_allThreadData[i]->allowToExecuteAgainInThisSimulationStep&&(!doAll)) )
                    {
                        _allThreadData[i]->allowToExecuteAgainInThisSimulationStep=false;
                        // Following is a special condition to support free-running mode:
                        if ( (!_allThreadData[i]->threadShouldRunFreely)&&(!_allThreadData[i]->threadSwitchShouldTriggerNoOtherThread) )
                        {
                            std::string tmp("==. In fiber/thread handling routine (fiberID/threadID: ");
                            tmp+=boost::lexical_cast<std::string>((unsigned long)_threadQueue[_threadQueue.size()-1]);
                            tmp+=")";
                            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

                            _unlock(8);
                            switchToThread((VTHREAD_ID_TYPE)_allThreadData[i]->threadID);
                            _lock(8);
                            i=0; // We re-check from the beginning
                            retVal++;
                        }
                    }
                }
            }
        }
        if (doAll)
            break;
    }
    _unlock(8);
    return(retVal);
}

CVThreadData* CThreadPool_old::getCurrentThreadData()
{
    return(getThreadData(VThread::getCurrentThreadId()));
}

CVThreadData* CThreadPool_old::getThreadData(VTHREAD_ID_TYPE threadId)
{
    CVThreadData* retVal=nullptr;
    _lock(9);
    for (size_t i=0;i<_allThreadData.size();i++)
    {
        if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,threadId))
        {
            retVal=_allThreadData[i];
            break;
        }
    }
    _unlock(9);
    return(retVal);
}

int CThreadPool_old::getThreadPoolThreadCount()
{ // Doesn't count the main thread!
    int retVal=0;
    _lock(10);
    if (_threadQueue.size()!=0)
    { // At least one thread was created
        retVal=int(_allThreadData.size())-1;
    }
    _unlock(10);
    return(retVal);
}

void CThreadPool_old::setSimulationEmergencyStop(bool stop)
{
    _lock(11);
    _threadShouldNotSwitch_override=true; // 21/6/2014
    _simulationEmergencyStopRequest=stop;
    _unlock(11);
}

bool CThreadPool_old::getSimulationEmergencyStop()
{
    _lock(12);

    // We first make sure the thread is not running in free mode. Otherwise we return false
    if (_inInterceptRoutine>0)
    {
        for (size_t i=0;i<_allThreadData.size();i++)
        {
            if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,VThread::getCurrentThreadId()))
            {
                if (_allThreadData[i]->threadShouldRunFreely)
                {
                    _unlock(12);
                    return(false);
                }
            }
        }
    }
    else
    {
        if (_threadQueue.size()!=0)
        {
            if (!VThread::areThreadIDsSame(_threadQueue[_threadQueue.size()-1],VThread::getCurrentThreadId()))
            {
                _unlock(12);
                return(false);
            }
        }
    }
    bool retVal=_simulationEmergencyStopRequest;
    _unlock(12);
    return(retVal);
}

void CThreadPool_old::forceAutomaticThreadSwitch_simulationEnding()
{
    _threadShouldNotSwitch_override=true; // 21/6/2014
}

void CThreadPool_old::setRequestSimulationStop(bool stop)
{
    _lock(13);
    if (stop)
    {
        _threadShouldNotSwitch_override=true; // 21/6/2014
        if (!_simulationStopRequest)
        {
            _simulationStopRequest=true;
            _simulationStopRequestTime=VDateTime::getTimeInMs();
        }
    }
    else
    {
        _threadShouldNotSwitch_override=false; // 21/6/2014
        _simulationStopRequest=false;
    }
    _unlock(13);
}

bool CThreadPool_old::getSimulationStopRequested()
{
    if (getSimulationEmergencyStop())
        return(true);
    _lock(14);

    // Do we really still need following?
    if (_threadQueue.size()!=0)
    {
        // Make sure the thread is not running in free mode. Otherwise we return false
        if (!VThread::areThreadIDsSame(_threadQueue[_threadQueue.size()-1],VThread::getCurrentThreadId()))
        {
            _unlock(14);
            return(false);
        }
    }

    bool retVal=_simulationStopRequest;
    _unlock(14);
    return(retVal);
}

bool CThreadPool_old::getSimulationStopRequestedAndActivated()
{
    if (_inInterceptRoutine>0)
        return(false);
    if (getSimulationEmergencyStop())
        return(true);
    _lock(15);

    // Do we really still need following?
    if (_threadQueue.size()!=0)
    {
        // Make sure the thread is not running in free mode. Otherwise we return false
        if (!VThread::areThreadIDsSame(_threadQueue[_threadQueue.size()-1],VThread::getCurrentThreadId()))
        {
            _unlock(15);
            return(false);
        }
    }

    bool retVal=false;
    if (_simulationStopRequest)
        retVal=VDateTime::getTimeDiffInMs(_simulationStopRequestTime)>=App::userSettings->threadedScriptsStoppingGraceTime;
    _unlock(15);
    return(retVal);
}

void CThreadPool_old::_lock(unsigned char debugInfo)
{
    _threadPoolMutex.lock("CThreadPool_old::_lock()");
    _lockStage++;
}

void CThreadPool_old::_unlock(unsigned char debugInfo)
{
    _lockStage--;
    _threadPoolMutex.unlock();
}

void CThreadPool_old::cleanUp()
{ // Make sure to release the main thread data:
    for (size_t i=0;i<_allThreadData.size();i++)
        delete _allThreadData[i];
}

bool CThreadPool_old::setThreadFreeMode(bool freeMode)
{
    if (_inInterceptRoutine>0)
        return(false);
    bool retVal=false;
    VTHREAD_ID_TYPE thisThreadID=VThread::getCurrentThreadId();
    _lock(17);
    if (freeMode)
    { // FREE MODE
        int fql=int(_threadQueue.size());
        if ( (fql>=2)&&VThread::areThreadIDsSame(_threadQueue[fql-1],thisThreadID) )
        {
            VTHREAD_ID_TYPE nextThreadID=_threadQueue[fql-2];
            CVThreadData* thisThreadData=nullptr;      
            CVThreadData* nextThreadData=nullptr;
            for (size_t i=0;i<_allThreadData.size();i++)
            {

                if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,thisThreadID))
                    thisThreadData=_allThreadData[i];
                if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,nextThreadID))
                    nextThreadData=_allThreadData[i];
            }

            std::string tmp("==f Starting thread free-mode (threadID: ");
            tmp+=boost::lexical_cast<std::string>((unsigned long)thisThreadData->threadID);
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

            _threadQueue.pop_back();
            thisThreadData->freeModeSavedThreadStartTime=_threadStartTime[_threadStartTime.size()-1];
            _threadStartTime.pop_back();
            thisThreadData->threadShouldRunFreely=true;
            thisThreadData->threadSwitchShouldTriggerNoOtherThread=true;
            nextThreadData->threadWantsResumeFromYield=true;
            _unlock(17);

            tmp="==f Started thread free-mode (threadID: ";
            tmp+=boost::lexical_cast<std::string>((unsigned long)thisThreadData->threadID);
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

            return(true);
        }
    }
    else
    { // NON-FREE MODE
        int fql=int(_threadQueue.size());
        if ( (fql>=1)&&(!VThread::areThreadIDsSame(_threadQueue[fql-1],thisThreadID)) )
        {
            CVThreadData* thisThreadData=nullptr;      
            for (size_t i=1;i<_allThreadData.size();i++)
            {
                if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,thisThreadID))
                    thisThreadData=_allThreadData[i];
            }

            std::string tmp("==e Ending thread free-mode (threadID: ");
            tmp+=boost::lexical_cast<std::string>((unsigned long)thisThreadData->threadID);
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

            thisThreadData->threadShouldRunFreely=false;
            _threadQueue.push_back((VTHREAD_ID_TYPE)thisThreadData->threadID);
            _threadStartTime.push_back((int)thisThreadData->freeModeSavedThreadStartTime);
            _unlock(17);
            switchBackToPreviousThread();

            tmp="==e Ended thread free-mode (threadID: ";
            tmp+=boost::lexical_cast<std::string>((unsigned long)thisThreadData->threadID);
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());

            return(true);
        }
    }

    if (!retVal)
    {
        if (freeMode)
        {
            std::string tmp("==x Failed starting thread free-mode (threadID: ");
            tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());
        }
        else
        {
            std::string tmp("==x Failed stopping thread free-mode (threadID: ");
            tmp+=boost::lexical_cast<std::string>((unsigned long)VThread::getCurrentThreadId());
            tmp+=")";
            App::logMsg(sim_verbosity_tracelua,tmp.c_str());
        }
    }

    _unlock(17);
    return(retVal);
}

int CThreadPool_old::getProcessorCoreAffinity()
{
    return(_processorCoreAffinity);
}

void CThreadPool_old::setProcessorCoreAffinity(int affinity)
{
    _processorCoreAffinity=affinity;
}

bool CThreadPool_old::isThreadInFreeMode()
{
    if (_inInterceptRoutine>0)
        return(false);
    if ( VThread::isCurrentThreadTheMainSimulationThread()||(!VThread::isSimulationMainThreadIdSet()) )
        return(false);
    bool retVal=false;
    _lock(18);
    int fql=int(_threadQueue.size());
    if (!VThread::areThreadIDsSame(_threadQueue[fql-1],VThread::getCurrentThreadId()))
        retVal=true;
    _unlock(18);
    return(retVal);
}

VTHREAD_RETURN_TYPE CThreadPool_old::_tmpCallback(VTHREAD_ARGUMENT_TYPE lpData)
{ // This callback is used to execute some functions via a specific thread
    void** valPtr=(void**)_tmpData;
    int callType=((int*)valPtr[0])[0];
    _tmpRetData=-1; // error
    if (callType==0)
    { // we want to call "script->callScriptFunction_DEPRECATED"
        CScriptObject* script=(CScriptObject*)valPtr[1];
        char* funcName=(char*)valPtr[2];
        SLuaCallBack* data=(SLuaCallBack*)valPtr[3];
        _tmpRetData=script->callScriptFunction_DEPRECATED(funcName,data);
    }
    if (callType==1)
    { // we want to call "script->callScriptFunction"
        CScriptObject* script=(CScriptObject*)valPtr[1];
        char* funcName=(char*)valPtr[2];
        CInterfaceStack* stack=(CInterfaceStack*)valPtr[3];
        _tmpRetData=script->callCustomScriptFunction(funcName,stack);
    }
    if (callType==2)
    { // we want to call "script->setScriptVariable"
        CScriptObject* script=(CScriptObject*)valPtr[1];
        char* varName=(char*)valPtr[2];
        CInterfaceStack* stack=(CInterfaceStack*)valPtr[3];
        _tmpRetData=script->setScriptVariable_old(varName,stack);
    }
    if (callType==3)
    { // we want to call "script->executeScriptString"
        CScriptObject* script=(CScriptObject*)valPtr[1];
        char* scriptString=(char*)valPtr[2];
        CInterfaceStack* stack=(CInterfaceStack*)valPtr[3];
        _tmpRetData=script->executeScriptString(scriptString,stack);
    }
    return(VTHREAD_RETURN_VAL);
}

bool CThreadPool_old::_interceptThread(VTHREAD_ID_TYPE theThreadToIntercept,VTHREAD_START_ADDRESS theCallback)
{
    _lock(1);
    bool retVal=false;
    VTHREAD_ID_TYPE fID=theThreadToIntercept;
    for (size_t i=0;i<_allThreadData.size();i++)
    {
        if (VThread::areThreadIDsSame(_allThreadData[i]->threadID,fID))
        {
            retVal=!_allThreadData[i]->threadShouldRunFreely;
            break;
        }
    }
    _unlock(1);
    if (retVal)
    {
        _threadInterceptIndex++;
        int v=_threadInterceptIndex;
        _threadInterceptCallback=theCallback;
        _threadToIntercept=theThreadToIntercept;

        while (v<=_threadInterceptIndex)
            VThread::switchThread();
        //_threadToIntercept is also set to zero by the thread itself
    }
    return(retVal);
}

int CThreadPool_old::callRoutineViaSpecificThread(VTHREAD_ID_TYPE theThread,void* data)
{
    _inInterceptRoutine++;
    _tmpData=data;
    int retVal=-1; // error
    if (CThreadPool_old::_interceptThread(theThread,_tmpCallback))
        retVal=_tmpRetData;
    _inInterceptRoutine--;
    return(retVal);
}
