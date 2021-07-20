
// This file requires some serious refactoring!

#include "simInternal.h"
#include "pageContainer.h"
#include "tt.h"
#include "sceneObjectOperations.h"
#include "simulation.h"
#include "sceneObject.h"
#include "gV.h"
#include "pluginContainer.h"
#include "fileOperations.h"
#include "simStrings.h"
#include "app.h"
#include "pageRendering.h"
#include "simFlavor.h"
#ifdef SIM_WITH_GUI
    #include "toolBarCommand.h"
#endif

CPageContainer::CPageContainer()
{
    _activePageIndex=0;
    rightMouseCaughtBy=-1;
    leftMouseCaughtBy=-1;
    prepareForPopupMenu=-1;
    rightMouseCaughtSoftDialog=false;
    leftMouseCaughtSoftDialog=false;
    _caughtElements=0;
    for (int i=0;i<PAGES_COUNT;i++)
        _allPages[i]=nullptr;
    #ifdef SIM_WITH_GUI
        setFocusObject(FOCUS_ON_PAGE);
    #endif
    setUpDefaultPages();
    removeAllPages();
    _initialValuesInitialized=false;
}

CPageContainer::~CPageContainer()
{ // beware, the current world could be nullptr
    removeAllPages();
}

void CPageContainer::emptySceneProcedure()
{
    removeAllPages();
}

int CPageContainer::getPageCount() const
{
    return(PAGES_COUNT);
}

void CPageContainer::initializeInitialValues(bool simulationAlreadyRunning,int initializeOnlyForThisNewObject)
{
    _initialValuesInitialized=true;
    _initialActivePageIndex=_activePageIndex;
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->initializeInitialValues(simulationAlreadyRunning,initializeOnlyForThisNewObject);
    }
}

void CPageContainer::simulationAboutToStart()
{
    initializeInitialValues(false,-1);
}

void CPageContainer::simulationEnded()
{
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    {
        _activePageIndex=_initialActivePageIndex;
    }
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->simulationEnded();
    }
    _initialValuesInitialized=false;
}


void CPageContainer::removeAllPages()
{
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            delete _allPages[i];
        _allPages[i]=nullptr;
    }
}
void CPageContainer::setUpDefaultPages(bool createASingleView)
{ // createASingleView is false by default
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
        {
            delete _allPages[i];
            _allPages[i]=nullptr;
        }
    }
    if (createASingleView)
    {
        CSPage* it=new CSPage(SINGLE_VIEW);
        _allPages[0]=it;
    }
    _activePageIndex=0;
}

CSPage* CPageContainer::getPage(int pageIndex) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( (pageIndex<0)||(pageIndex>=PAGES_COUNT) )
        return(nullptr); // Invalid view number
    return(_allPages[pageIndex]);   
}

int CPageContainer::getMainCameraHandle() const
{
    int retVal=-1;
#ifdef SIM_WITH_GUI
    CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
    if (page!=nullptr)
    {
        for (size_t i=0;i<10;i++)
        {
            CSView* view=page->getView(i);
            if (view!=nullptr)
            {
                CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(view->getLinkedObjectID());
                if (cam!=nullptr)
                {
                    retVal=cam->getObjectHandle();
                    break;
                }
            }
        }
    }
#endif
    return(retVal);
}

void CPageContainer::announceObjectWillBeErased(int objectID)
{ // Never called from copy buffer!
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->announceObjectWillBeErased(objectID);
    }
}

void CPageContainer::setPageSizeAndPosition(int sizeX,int sizeY,int posX,int posY)
{
    _pageSize[0]=sizeX;
    _pageSize[1]=sizeY;
    _pagePosition[0]=posX;
    _pagePosition[1]=posY;
    // We set the view position and size for all soft dialogs:
    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->setViewSizeAndPosition(sizeX,sizeY,posX,posY);
    // We set the view position and size for all views:
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->setPageSizeAndPosition(sizeX,sizeY,posX,posY);
    }
}

void CPageContainer::removePage(int pageIndex)
{
    if ( (pageIndex<0)||(pageIndex>=PAGES_COUNT) )
        return;
    if (_allPages[pageIndex]!=nullptr)
    {
        delete _allPages[pageIndex];
        _allPages[pageIndex]=nullptr;
    }
}

void CPageContainer::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            for (int i=0;i<PAGES_COUNT;i++)
            {
                if (_allPages[i]!=nullptr)
                {
                    ar.storeDataName("Vwo");
                    ar.setCountingMode();
                    _allPages[i]->serialize(ar);
                    if (ar.setWritingMode())
                        _allPages[i]->serialize(ar);
                }
                else
                {
                    ar.storeDataName("Nul");
                    ar << (int)0;
                    ar.flush();
                }
            }

            ar.storeDataName("Avi");
            ar << _activePageIndex;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            removeAllPages();
            int viewCounter=0;
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Avi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _activePageIndex;
                        if (_activePageIndex>=PAGES_COUNT)
                            _activePageIndex=0;
                        App::setToolbarRefreshFlag();
                    }
                    if (theName.compare("Vwo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CSPage* theView=new CSPage(0);
                        theView->serialize(ar);
                        if (viewCounter<PAGES_COUNT)
                            _allPages[viewCounter]=theView;
                        else
                            delete theView;
                        viewCounter++;
                    }
                    if (theName.compare("Nul")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int tmp;
                        ar >> tmp;
                        viewCounter++;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_int("currentPage",_activePageIndex);

            for (int i=0;i<PAGES_COUNT;i++)
            {
                ar.xmlPushNewNode("pageWrapper");
                if (_allPages[i]!=nullptr)
                {
                    ar.xmlPushNewNode("page");
                    _allPages[i]->serialize(ar);
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
        }
        else
        {
            ar.xmlGetNode_int("currentPage",_activePageIndex);
            int viewCounter=0;
            if (ar.xmlPushChildNode("pageWrapper",false))
            {
                while (true)
                {
                    if (ar.xmlPushChildNode("page",false))
                    {
                        CSPage* theView=new CSPage(0);
                        theView->serialize(ar);
                        _allPages[viewCounter]=theView;
                        ar.xmlPopNode();
                    }
                    viewCounter++;
                    if (!ar.xmlPushSiblingNode("pageWrapper",false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CPageContainer::performObjectLoadingMapping(const std::vector<int>* map)
{
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->performObjectLoadingMapping(map);
    }
}

bool CPageContainer::processCommand(int commandID,int viewIndex)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if ( (viewIndex<0)||(viewIndex>=PAGES_COUNT) )
        return(false);
    if (_allPages[viewIndex]!=nullptr)
        return(false);

    if ( (commandID==PAGE_CONT_FUNCTIONS_CREATE_SINGLE_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_FOUR_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_SIX_VIEW_TYPE_PCCMD)||
         (commandID==PAGE_CONT_FUNCTIONS_CREATE_EIGHT_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_VIEW_TYPE_PCCMD)||
         (commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_3_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_3_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_3_VIEW_TYPE_PCCMD)||
         (commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_3_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_4_VIEW_TYPE_PCCMD)||(commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_4_VIEW_TYPE_PCCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CSPage* it=nullptr;
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_SINGLE_VIEW_TYPE_PCCMD)
                it=new CSPage(SINGLE_VIEW);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_FOUR_VIEW_TYPE_PCCMD)
                it=new CSPage(FOUR_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_SIX_VIEW_TYPE_PCCMD)
                it=new CSPage(SIX_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_EIGHT_VIEW_TYPE_PCCMD)
                it=new CSPage(EIGHT_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_VIEW_TYPE_PCCMD)
                it=new CSPage(HORIZONTALLY_DIVIDED);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_VIEW_TYPE_PCCMD)
                it=new CSPage(VERTICALLY_DIVIDED);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_3_VIEW_TYPE_PCCMD)
                it=new CSPage(HORIZONTALLY_DIVIDED_3);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_3_VIEW_TYPE_PCCMD)
                it=new CSPage(VERTICALLY_DIVIDED_3);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_3_VIEW_TYPE_PCCMD)
                it=new CSPage(HORIZONTAL_1_PLUS_3_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_3_VIEW_TYPE_PCCMD)
                it=new CSPage(VERTICAL_1_PLUS_3_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_4_VIEW_TYPE_PCCMD)
                it=new CSPage(HORIZONTAL_1_PLUS_4_VIEWS);
            if (commandID==PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_4_VIEW_TYPE_PCCMD)
                it=new CSPage(VERTICAL_1_PLUS_4_VIEWS);
            App::logMsg(sim_verbosity_msgs,IDSNS_CREATED_VIEWS);
            if (it!=nullptr)
            {
                it->setPageSizeAndPosition(_pageSize[0],_pageSize[1],_pagePosition[0],_pagePosition[1]);
                _allPages[viewIndex]=it;
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(viewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    return(false);
}

#ifdef SIM_WITH_GUI
void CPageContainer::renderCurrentPage(bool hideWatermark)
{
    TRACE_INTERNAL;
    CSPage* it=getPage(_activePageIndex);
    displayContainerPage(it,_pagePosition,_pageSize);

    // Now we have to clear all mouseJustWentDown and mouseJustWentUp flag in case
    // it was not processed
    for (int  i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->clearAllMouseJustWentDownAndUpFlags();
    }

    displayContainerPageOverlay(_pagePosition,_pageSize,_activePageIndex,focusObject);

    if ( (!hideWatermark)&&(App::mainWindow!=nullptr)&&(!App::mainWindow->simulationRecorder->getIsRecording()) )
    {
        int tagId=CSimFlavor::getIntVal(0);
        // if compiling CoppeliaSim yourself, and using the mesh calculation plugin, you must use the EDU_TAG below!
        if (tagId==0)
            displayContainerPageWatermark(_pagePosition,_pageSize,EDU_TAG);
        if (tagId==1)
            displayContainerPageWatermark(_pagePosition,_pageSize,EVAL_TAG);
    }
}

int CPageContainer::getActivePageIndex() const
{
    return(_activePageIndex);
}

void CPageContainer::setActivePage(int pageIndex)
{
    if ( (pageIndex<PAGES_COUNT)&&(pageIndex>=0) )
    {
        _activePageIndex=pageIndex;
        clearAllLastMouseDownViewIndex(); // foc
        App::setToolbarRefreshFlag();
    }
}

void CPageContainer::addPageMenu(VMenu* menu)
{
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_SINGLE_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_SINGLE_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_TWO_HORIZ_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_TWO_VERT_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_3_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_THREE_HORIZ_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_3_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_THREE_VERT_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_3_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_HORIZONTAL_ONE_PLUS_THREE_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_3_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_VERTICAL_ONE_PLUS_THREE_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_HORIZONTAL_1_PLUS_4_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_HORIZONTAL_ONE_PLUS_FOUR_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_VERTICAL_1_PLUS_4_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_VERTICAL_ONE_PLUS_FOUR_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_FOUR_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_FOUR_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_SIX_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_SIX_MENU_ITEM);
    menu->appendMenuItem(true,false,PAGE_CONT_FUNCTIONS_CREATE_EIGHT_VIEW_TYPE_PCCMD,IDS_VIEW_TYPE_EIGHT_MENU_ITEM);
}

bool CPageContainer::getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const
{ // NOT FULLY IMPLEMENTED! objType=-1 --> not supported, 0 --> hierarchy, 1 --> 3DViewable
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(false);
    // The position is in this window zone
    if ((App::currentWorld->buttonBlockContainer!=nullptr)&&App::currentWorld->buttonBlockContainer->mouseDownTest(x,y,_activePageIndex))
    { // not yet supported
        objType=-1;
        return(true);
    }
    const CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->getMouseRelPosObjectAndViewSize(x,y,relPos,objType,objID,vSize,viewIsPerspective))
            return(true); // we are in this view
    }
    return(false);
}

bool CPageContainer::leftMouseButtonDown(int x,int y,int selectionStatus)
{
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(false);
    // The mouse went down in this window zone
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->clearCaughtElements(0xffff-sim_left_button);
    }
    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->clearCaughtElements(0xffff-sim_left_button);

    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    leftMouseCaughtBy=-1;
    leftMouseCaughtSoftDialog=false;
    if ((App::currentWorld->buttonBlockContainer!=nullptr)&&App::currentWorld->buttonBlockContainer->mouseDown(mouseRelativePosition[0],mouseRelativePosition[1],_activePageIndex,selectionStatus))
    {
        setFocusObject(FOCUS_ON_SOFT_DIALOG);
        return(true);
    }
    setFocusObject(FOCUS_ON_PAGE);
    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->leftMouseButtonDown(x,y,selectionStatus))
        { // The mouse was caught by this view:
            return(true);
        }
    }
    // Here we could handle mouse catch for the view container (not handled now)
    return(false);
}

void CPageContainer::clearAllLastMouseDownViewIndex()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<PAGES_COUNT;i++)
    {
        CSPage* p=_allPages[i];
        if (p!=nullptr)
            p->clearLastMouseDownViewIndex();
    }
}

int CPageContainer::getCursor(int x,int y) const
{
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(-1);
    if ((App::currentWorld->buttonBlockContainer!=nullptr)&&App::currentWorld->buttonBlockContainer->mouseDownTest(x,y,_activePageIndex))
        return(-1);
    CSPage* it=getPage(_activePageIndex);
    if (it==nullptr)
        return(-1); // The active view doesn't exist!
    return(it->getCursor(x,y));
}

void CPageContainer::leftMouseButtonUp(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        if (App::currentWorld->buttonBlockContainer->getCaughtElements()&sim_left_button)
            App::currentWorld->buttonBlockContainer->mouseUp(mouseRelativePosition[0],mouseRelativePosition[1],_activePageIndex);
    }

    CSPage* it=getPage(_activePageIndex);
    if ( (it!=nullptr)&&(it->getCaughtElements()&sim_left_button) )
        it->leftMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1]);
}

void CPageContainer::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int bts=sim_right_button|sim_middle_button|sim_left_button;
    if (App::userSettings->navigationBackwardCompatibility)
        bts=sim_right_button|sim_left_button;

    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        if (!passiveAndFocused)
        {
            if (App::currentWorld->buttonBlockContainer->getCaughtElements()&bts)
                App::currentWorld->buttonBlockContainer->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1]);
        }
        else
            App::currentWorld->buttonBlockContainer->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1]);
    }
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
        {
            if (!passiveAndFocused)
            {
                if (_allPages[i]->getCaughtElements()&bts)
                    _allPages[i]->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
            }
            else
            {
                if (i==_activePageIndex)
                    _allPages[i]->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
            }
        }
    }
}

int CPageContainer::modelDragMoveEvent(int x,int y,C3Vector* desiredModelPosition)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
        return(it->modelDragMoveEvent(mouseRelativePosition[0],mouseRelativePosition[1],desiredModelPosition));
    return(0);
}

void CPageContainer::mouseWheel(int deltaZ,int x,int y)
{
    // We sent this event only to the active view if no view was caught:
    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
        it->mouseWheel(deltaZ,x,y);
}

void CPageContainer::looseFocus()
{
    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->looseFocus();
    setFocusObject(FOCUS_ON_UNKNOWN_OBJECT);
}

int CPageContainer::getFocusObject() const
{
    return(focusObject);
}

void CPageContainer::setFocusObject(int obj)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    focusObject=obj;
    if (focusObject==FOCUS_ON_PAGE)
    {
        if (App::currentWorld->buttonBlockContainer!=nullptr)
        {
            App::currentWorld->buttonBlockContainer->setEditBoxEdition(-1,-1,false);
            App::currentWorld->buttonBlockContainer->looseFocus();
        }
    }
}

void CPageContainer::keyPress(int key,QWidget* mainWindow)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ((focusObject==FOCUS_ON_SOFT_DIALOG)&&(App::currentWorld->buttonBlockContainer!=nullptr))
    {
        App::currentWorld->buttonBlockContainer->onKeyDown(key);
        return;
    }

    if ( (key==CTRL_V_KEY)||(key==DELETE_KEY)||(key==BACKSPACE_KEY)||(key==CTRL_X_KEY)||(key==CTRL_C_KEY)||(key==ESC_KEY)||(key==CTRL_A_KEY)||(key==CTRL_Y_KEY)||(key==CTRL_Z_KEY) )
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->editModeContainer->keyPress(key)) )
            CSceneObjectOperations::keyPress(key); // the key press was not for the edit mode
        return;
    }

    if ( (key==CTRL_S_KEY)||(key==CTRL_O_KEY)||(key==CTRL_W_KEY)||(key==CTRL_Q_KEY)||(key==CTRL_N_KEY) )
    {
        CFileOperations::keyPress(key);
        return;
    }

    if (key==CTRL_SPACE_KEY)
    {
        App::currentWorld->simulation->keyPress(key);
        return;
    }

    if (key==CTRL_E_KEY)
    {
        App::worldContainer->keyPress(key);
        return;
    }
    if ( (key==CTRL_D_KEY)||(key==CTRL_G_KEY) )
    {
        App::mainWindow->dlgCont->keyPress(key);
        return;
    }

    int flags=0;
    if (App::mainWindow!=nullptr)
    {
        if (App::mainWindow->getKeyDownState()&1)
            flags|=1;
        if (App::mainWindow->getKeyDownState()&2)
            flags|=2;
    }
    int data[4]={key,flags,0,0};
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_keypress,data,nullptr,nullptr);
    delete[] (char*)retVal;
    data[0]=key;
    data[1]=flags;
    retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_keypress,data,nullptr,nullptr); // for backward compatibility
    delete[] (char*)retVal;
    App::currentWorld->outsideCommandQueue->addCommand(sim_message_keypress,key,flags,0,0,nullptr,0);

    App::worldContainer->setModificationFlag(1024); // key was pressed
}

bool CPageContainer::rightMouseButtonDown(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( (x<0)||(y<0)||(x>_pageSize[0])||(y>_pageSize[1]) )
        return(false);
    // The mouse went down in this window zone
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->clearCaughtElements(0xffff-sim_right_button);
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->clearCaughtElements(0xffff-sim_right_button);
    }
    prepareForPopupMenu=-1;
    // First we check the soft dialogs:

    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        // Following 2 lines should be replaced with the right button down handling routine (when it exists)! (and then return true!)
        if (App::currentWorld->buttonBlockContainer->mouseDownTest(mouseRelativePosition[0],mouseRelativePosition[1],_activePageIndex))
            return(false);
    }

    setFocusObject(FOCUS_ON_PAGE);
    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->rightMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1]))
            return(true); // The active view caught that event!
    }
    else
    { // Here we prepare info for the popup menu in the case no view exists for that index:
        _caughtElements|=sim_right_button; // we catch the right button
        prepareForPopupMenu=_activePageIndex;
        return(true); // We might display a popup menu to allow the user to create a view for that _activePageIndex
    }
    return(false); // Nothing caught that action
}

void CPageContainer::rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;


    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        // ROUTINE DOES NOT YET EXIST!
    }
    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->getCaughtElements()&sim_right_button)
        {
            if (it->rightMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1],absX,absY,mainWindow))
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=REMOVE_CURRENT_PAGE_CMD;
                App::appendSimulationThreadCommand(cmd);
            }
        }
    }
    else
    {
        if (_caughtElements&sim_right_button)
        {
            if ( (prepareForPopupMenu>=0)&&(prepareForPopupMenu<PAGES_COUNT)&&(prepareForPopupMenu==_activePageIndex) )
            {
                // Did the mouse go up in this zone?
                if ( (x>=0)&&(y>=0)&&(x<=_pageSize[0])&&(y<=_pageSize[1]) )
                { // Yes! We display a popup menu to allow the user to create a new view at that index:
                    if (App::operationalUIParts&sim_gui_popups)
                    { // Default popups
                        VMenu mainMenu=VMenu();
                        addPageMenu(&mainMenu);
                        int command=mainMenu.trackPopupMenu();
                        processCommand(command,_activePageIndex);
                    }
                }
            }
        }
    }
    prepareForPopupMenu=-1;
}

bool CPageContainer::middleMouseButtonDown(int x,int y)
{
    if ( (x<0)||(y<0)||(x>_pageSize[0])||(y>_pageSize[1]) )
        return(false);

    // The mouse went down in this window zone
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->clearCaughtElements(0xffff-sim_middle_button);

    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->clearCaughtElements(0xffff-sim_middle_button);
    }

    // First we check the soft dialogs:
    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        // Following 2 lines should be replaced with the right button down handling routine (when it exists)! (and then return true!)
        if (App::currentWorld->buttonBlockContainer->mouseDownTest(mouseRelativePosition[0],mouseRelativePosition[1],_activePageIndex))
            return(false);
    }

    setFocusObject(FOCUS_ON_PAGE);
    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->middleMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1]))
            return(true); // The active view caught that event!
    }
    return(false); // Nothing caught that action
}

void CPageContainer::middleMouseButtonUp(int x,int y)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;


    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        // ROUTINE DOES NOT YET EXIST!
    }

    CSPage* it=getPage(_activePageIndex);
    if (it!=nullptr)
    {
        if (it->getCaughtElements()&sim_middle_button)
            it->middleMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1]);
    }
}

bool CPageContainer::leftMouseButtonDoubleClick(int x,int y,int selectionStatus)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( (x<0)||(y<0)||(x>_pageSize[0])||(y>_pageSize[1]) )
        return(false);
    // The mouse went down in this window zone
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    // Soft dialogs:
    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        if (App::currentWorld->buttonBlockContainer->leftMouseButtonDoubleClick(x,y,_activePageIndex))
        {
            setFocusObject(FOCUS_ON_SOFT_DIALOG);
            return(true);
        }
    }
    setFocusObject(FOCUS_ON_PAGE);
    { // If dialog focus could be lost:
        CSPage* it=getPage(_activePageIndex);
        if (it!=nullptr)
            return(it->leftMouseButtonDoubleClick(x,y,selectionStatus));
    }
    return(false); // Not processed
}

int CPageContainer::getCaughtElements() const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int retVal=0;
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            retVal|=_allPages[i]->getCaughtElements();
    }
    if (App::currentWorld->buttonBlockContainer!=nullptr)
        retVal|=App::currentWorld->buttonBlockContainer->getCaughtElements();
    retVal|=_caughtElements;
    return(retVal);
}

void CPageContainer::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<PAGES_COUNT;i++)
    {
        if (_allPages[i]!=nullptr)
            _allPages[i]->clearCaughtElements(keepMask);
    }
    if (App::currentWorld->buttonBlockContainer!=nullptr)
        App::currentWorld->buttonBlockContainer->clearCaughtElements(keepMask);
    _caughtElements&=keepMask;
}

#endif
