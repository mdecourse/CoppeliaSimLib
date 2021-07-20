#include "simInternal.h"
#include "undoBufferCont.h"
#include "pluginContainer.h"
#include "vDateTime.h"
#include "app.h"
#include "simStrings.h"

CUndoBufferCont::CUndoBufferCont()
{
    _commonInit();
}

CUndoBufferCont::~CUndoBufferCont()
{ // beware, the current world could be nullptr
    for (size_t i=0;i<_buffers.size();i++)
        delete _buffers[i];
    undoBufferArrays.clearAll();
}

int CUndoBufferCont::getNextBufferId()
{
    return(_nextBufferId);
}

void CUndoBufferCont::emptySceneProcedure()
{
    _commonInit();
}

void CUndoBufferCont::_commonInit()
{
    _currentStateIndex=-1;
    for (int i=0;i<int(_buffers.size());i++)
        delete _buffers[i];
    _buffers.clear();
    undoBufferArrays.clearAll();
    _announceChangeStartCalled=false;
    _announceChangeGradualCalledTime=-1;
    _sceneSaveMightBeNeeded=false;
    _undoPointSavingOrRestoringUnderWay=false;
    _inUndoRoutineNow=false;
    _nextBufferId=0;
}

bool CUndoBufferCont::isUndoSavingOrRestoringUnderWay()
{
    TRACE_INTERNAL;
    return(_undoPointSavingOrRestoringUnderWay);
}

bool CUndoBufferCont::isSceneSaveMaybeNeededFlagSet()
{
    bool codeEditorChange=false;
#ifdef SIM_WITH_GUI
    codeEditorChange=( (App::mainWindow!=nullptr)&&App::mainWindow->codeEditorContainer->hasSomethingBeenModifiedInCurrentScene());
#endif
    return(_sceneSaveMightBeNeeded||codeEditorChange);
}

void CUndoBufferCont::clearSceneSaveMaybeNeededFlag()
{
    _sceneSaveMightBeNeeded=false;
}

bool CUndoBufferCont::announceChange()
{
    TRACE_INTERNAL;
    if (!_inUndoRoutineNow)
    {
        bool retVal=memorizeState();
        _announceChangeGradualCalledTime=-1;
        return(retVal);
    }
    return(false);
}

void CUndoBufferCont::announceChangeStart()
{
    TRACE_INTERNAL;
    if (!_inUndoRoutineNow)
        _announceChangeStartCalled=true;
}

void CUndoBufferCont::announceChangeEnd()
{
    TRACE_INTERNAL;
    if (!_inUndoRoutineNow)
    {
        if (_announceChangeStartCalled)
        {
            memorizeState();
            _announceChangeGradualCalledTime=-1;
        }
        _announceChangeStartCalled=false;
    }
}

void CUndoBufferCont::announceChangeGradual()
{
    TRACE_INTERNAL;
    if (!_inUndoRoutineNow)
    {
        if (_announceChangeGradualCalledTime==-1)
            _announceChangeGradualCalledTime=VDateTime::getTimeInMs();
    }
}

void CUndoBufferCont::memorizeStateIfNeeded()
{
    TRACE_INTERNAL;
    if (!_inUndoRoutineNow)
    {
        if (_announceChangeGradualCalledTime==-1)
            return;
        if (VDateTime::getTimeDiffInMs(_announceChangeGradualCalledTime)>5000)
        {
            memorizeState();
            _announceChangeGradualCalledTime=-1;
        }
    }
}

void CUndoBufferCont::emptyRedoBuffer()
{
    TRACE_INTERNAL;
    while (int(_buffers.size())>_currentStateIndex+1)
    {
        undoBufferArrays.removeDependenciesFromUndoBufferId(_buffers[_buffers.size()-1]->getBufferId());
        delete _buffers[_buffers.size()-1];
        _buffers.pop_back();
        App::setToolbarRefreshFlag();
    }
}

bool CUndoBufferCont::memorizeState()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (_inUndoRoutineNow)
        return(false);
    if (!App::currentWorld->simulation->isSimulationStopped())
        return(false);
    if (App::mainWindow==nullptr)
        return(false); // we are in headless mode

    // If we arrived here, we might have something to save:

    if (!_isGoodToMemorizeUndoOrRedo())
    { // we cannot memorize anything now...
        _sceneSaveMightBeNeeded=true; // actually we don't know if this is really the case, but since we are not authorized to check right now, this is safer!
        return(false);
    }

    App::currentWorld->setEnableRemoteWorldsSync(false);
    bool retVal=true; // means the scene changed.. we modify this variable accordingly later down

    static int tooLongExecutionCount=0;
    int startTime=VDateTime::getTimeInMs();

    if (App::mainWindow!=nullptr)
        App::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();

    std::vector<char> newBuff;
    CSer serObj(newBuff,CSer::filetype_csim_bin_scene_buff);
    serObj.disableCountingModeExceptForExceptions();

    serObj.writeOpenBinary(false); // we don't wanna compression
    _undoPointSavingOrRestoringUnderWay=true;
    CUndoBufferCameras* cameraBuffers=new CUndoBufferCameras();
    cameraBuffers->storeCameras();
    App::currentWorld->saveScene(serObj); // This takes the 90% of time of the whole routine
    cameraBuffers->releaseCameras();
    _undoPointSavingOrRestoringUnderWay=false;
    serObj.writeClose();

    CUndoBuffer* it=new CUndoBuffer(newBuff,_nextBufferId++,cameraBuffers);
    if (_currentStateIndex==-1)
    { // first buffer, we just add it
        _buffers.push_back(it);
        _currentStateIndex=0;
    }
    else
    { // We check with previous buffer:
        std::vector<char> prevBuff;
        _getFullBuffer(_currentStateIndex,prevBuff);
        if (it->finalize(prevBuff))
        { // different from previous, we remove forward buffers and add this one:
            while (int(_buffers.size())>_currentStateIndex+1)
            {
                undoBufferArrays.removeDependenciesFromUndoBufferId(_buffers[_buffers.size()-1]->getBufferId());
                delete _buffers[_buffers.size()-1];
                _buffers.pop_back();
            }
            _buffers.push_back(it);
            _currentStateIndex++;
        }
        else
        {
            undoBufferArrays.removeDependenciesFromUndoBufferId(it->getBufferId());
            delete it; // same as previous, we delete it
            retVal=false;
        }
    }

    std::vector<char> fullBuff;
    while ( (_getUsedMemory()+undoBufferArrays.getMemorySizeInBytes()>App::userSettings->undoRedoMaxBufferSize)||(int(_buffers.size())>App::userSettings->undoRedoLevelCount) )
    { // We have to remove a few states at the beginning
        if (int(_buffers.size())<3)
            break; // at least 3 states!
        _getFullBuffer(1,fullBuff);
        CUndoBuffer* it=_buffers[1];
        it->updateWithFullBuffer(fullBuff);
        undoBufferArrays.removeDependenciesFromUndoBufferId(_buffers[0]->getBufferId());
        delete _buffers[0];
        _buffers.erase(_buffers.begin());
        _currentStateIndex--;
    }
    if (VDateTime::getTimeDiffInMs(startTime)>App::userSettings->timeInMsForUndoRedoTooLongWarning)
        tooLongExecutionCount++;
    else
        tooLongExecutionCount=0;

    if (tooLongExecutionCount>=5)
    {
        static bool displayedMessage=false;
        if (!displayedMessage)
        {
            if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,"Undo / redo",IDS_INFO_UNDO_REDO_TAKES_TOO_LONG,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_NO))
                App::userSettings->setUndoRedoEnabled(false);
            displayedMessage=true;
        }
        tooLongExecutionCount=0;
    }

    App::setToolbarRefreshFlag();

    _sceneSaveMightBeNeeded=_sceneSaveMightBeNeeded||retVal;
    App::currentWorld->setEnableRemoteWorldsSync(true);
    return(retVal);
#else
    return(false);
#endif
}

bool CUndoBufferCont::_isGoodToMemorizeUndoOrRedo()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (!App::currentWorld->simulation->isSimulationStopped())
        return(false);
    if (App::mainWindow==nullptr)
        return(false);
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return(false);
    if (App::mainWindow->oglSurface->isPageSelectionActive())
        return(false);
    if (App::mainWindow->oglSurface->isViewSelectionActive())
        return(false);
    if (!App::userSettings->getUndoRedoEnabled())
        return(false);
    return(true);
#else
    return(false);
#endif
}

bool CUndoBufferCont::canUndo()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (App::mainWindow==nullptr)
        return(false); // we are in headless mode
    if (!_isGoodToMemorizeUndoOrRedo())
        return(false);
    return(_currentStateIndex>0);
#else
    return(false);
#endif
}

bool CUndoBufferCont::canRedo()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (App::mainWindow==nullptr)
        return(false); // we are in headless mode
    if (!_isGoodToMemorizeUndoOrRedo())
        return(false);
    return(_currentStateIndex<=(int(_buffers.size())-2));
#else
    return(false);
#endif
}

void CUndoBufferCont::undo()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (_inUndoRoutineNow)
        return;
    if (App::mainWindow==nullptr)
        return; // we are in headless mode

    if (App::mainWindow->codeEditorContainer->areSceneEditorsOpen())
    {
        App::uiThread->messageBox_information(App::mainWindow,IDSN_UNDO_REDO,IDS_UNDO_REDO_WITH_OPEN_SCRIPT_EDITOR_MESSAGE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        return;
    }
    _inUndoRoutineNow=true;
    // 1. We memorize this position: NOOOOOOO!!! Can cause subtle errors!! 
    if (_announceChangeGradualCalledTime!=-1)
    {
        //  memorizeState();
        _announceChangeGradualCalledTime=-1;
    }
    if (_currentStateIndex<1)
    {
        _inUndoRoutineNow=false;
        return; // nothing to undo
    }

    // 2. We go back:
    _currentStateIndex--;
    std::vector<char> theBuff;
    CUndoBufferCameras* cameraBuffers=_getFullBuffer(_currentStateIndex,theBuff);
    // 3. Load the buffer:

    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_abouttoundo,nullptr,nullptr,nullptr);
    delete[] (char*)returnVal;

    _rememberSelectionState();
    cameraBuffers->preRestoreCameras();

    App::currentWorld->sceneObjects->deselectObjects();
    App::currentWorld->simulation->stopSimulation(); // should be anyway stopped!
    App::currentWorld->clearScene(false);

    CSer serObj(theBuff,CSer::filetype_csim_bin_scene_buff);
    int serializationVersion;
    unsigned short dum0;
    int dum1;
    char dum2;
    serObj.readOpenBinary(serializationVersion,dum0,dum1,dum2,false);
    _undoPointSavingOrRestoringUnderWay=true;

    App::currentWorld->loadScene(serObj,true);
    cameraBuffers->restoreCameras();

    _undoPointSavingOrRestoringUnderWay=false;
    serObj.readClose();
    if (App::mainWindow!=nullptr)
        App::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the correct dimensions

    // 4. We select previously selected objects:
    _restoreSelectionState();

    _inUndoRoutineNow=false;
    
    returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_undoperformed,nullptr,nullptr,nullptr);
    delete[] (char*)returnVal;
    App::worldContainer->setModificationFlag(16); // undo called

    // 5. Dialog refresh:
    App::setFullDialogRefreshFlag();
    App::setToolbarRefreshFlag();
#endif
}

void CUndoBufferCont::redo()
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if (_currentStateIndex>(int(_buffers.size())-2))
        return; // nothing to redo
    if (App::mainWindow==nullptr)
        return; // we are in headless mode
    if (App::mainWindow->codeEditorContainer->areSceneEditorsOpen())
    {
        App::uiThread->messageBox_information(App::mainWindow,IDSN_UNDO_REDO,IDS_UNDO_REDO_WITH_OPEN_SCRIPT_EDITOR_MESSAGE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        return;
    }
    // 2. We go forward:
    _currentStateIndex++;
    std::vector<char> theBuff;
    CUndoBufferCameras* cameraBuffers=_getFullBuffer(_currentStateIndex,theBuff);

    // 3. Load the buffer:

    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_abouttoredo,nullptr,nullptr,nullptr);
    delete[] (char*)returnVal;

    _rememberSelectionState();
    cameraBuffers->preRestoreCameras();

    App::currentWorld->sceneObjects->deselectObjects();
    App::currentWorld->simulation->stopSimulation(); // should be anyway stopped!
    App::currentWorld->clearScene(false);

    CSer serObj(theBuff,CSer::filetype_csim_bin_scene_buff);
    int serializationVersion;
    unsigned short dum0;
    int dum1;
    char dum2;
    serObj.readOpenBinary(serializationVersion,dum0,dum1,dum2,false);
    _undoPointSavingOrRestoringUnderWay=true;

    App::currentWorld->loadScene(serObj,true);
    cameraBuffers->restoreCameras();

    _undoPointSavingOrRestoringUnderWay=false;
    serObj.readClose();
    if (App::mainWindow!=nullptr)
        App::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the correct dimensions


    // 4. We select previously selected objects:
    _restoreSelectionState();

    returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_redoperformed,nullptr,nullptr,nullptr);
    delete[] (char*)returnVal;
    App::worldContainer->setModificationFlag(32); // redo called

    // 5. Dialog refresh:
    App::setFullDialogRefreshFlag();
    App::setToolbarRefreshFlag();
#endif
}

CUndoBufferCameras* CUndoBufferCont::_getFullBuffer(int index,std::vector<char>& fullBuff)
{
    TRACE_INTERNAL;
    if ( (index>=int(_buffers.size()))||(index<0) )
    {
        fullBuff.clear();
        return(nullptr);
    }
    CUndoBufferCameras* retVal=nullptr;
    if (index==0)
    { // the first one
        retVal=_buffers[index]->getCameraBuffers();
        _buffers[index]->getRestored(nullptr,fullBuff);
    }
    else
    {
        std::vector<char> prevBuff;
        _getFullBuffer(index-1,prevBuff);
        retVal=_buffers[index]->getCameraBuffers();
        _buffers[index]->getRestored(&prevBuff,fullBuff);
    }
    return(retVal);
}

int CUndoBufferCont::_getUsedMemory()
{
    TRACE_INTERNAL;
    int retVal=0;
    for (size_t i=0;i<_buffers.size();i++)
        retVal+=int(_buffers[i]->buffer.size());
    return (retVal);
}

void CUndoBufferCont::_rememberSelectionState()
{
    TRACE_INTERNAL;
    _selectionState.clear();
    std::vector<CSceneObject*> sel;
    App::currentWorld->sceneObjects->getSelectedObjects(sel);
    for (size_t i=0;i<sel.size();i++)
        _selectionState.push_back(sel[i]->getObjectAlias_fullPath());
}

void CUndoBufferCont::_restoreSelectionState()
{
    TRACE_INTERNAL;
    for (size_t i=0;i<_selectionState.size();i++)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,_selectionState[i].c_str(),-1,nullptr);
        if (obj!=nullptr)
            App::currentWorld->sceneObjects->addObjectToSelection(obj->getObjectHandle());
    }
    _selectionState.clear();
}

