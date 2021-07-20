#pragma once

#include "3Vector.h"
#include "4Vector.h"
#include "7Vector.h"
#include "sceneObject.h"

struct SUIThreadCommand
{
    int cmdId;
    std::vector<bool> boolParams;
    std::vector<int> intParams;
    std::vector<unsigned int> uintParams;
    std::vector<float> floatParams;
    std::vector<void*> objectParams;
    std::vector<std::string> stringParams;
    std::vector<C3Vector> posParams;
    std::vector<C4Vector> quatParams;
    std::vector<C7Vector> transfParams;
};

enum {
        VMESSAGEBOX_OKELI           =0,
        VMESSAGEBOX_YES_NO          =1,
        VMESSAGEBOX_YES_NO_CANCEL   =2,
        VMESSAGEBOX_OK_CANCEL       =3,
};

enum{
        VMESSAGEBOX_REPLY_CANCEL    =0,
        VMESSAGEBOX_REPLY_NO        =1,
        VMESSAGEBOX_REPLY_YES       =2,
        VMESSAGEBOX_REPLY_OK        =3,
        VMESSAGEBOX_REPLY_ERROR     =4,
};

enum {  OPEN_OR_CLOSE_UITHREADCMD=0,

        CREATE_DEFAULT_MENU_BAR_UITHREADCMD,
        KEEP_THUMBNAIL_QUESTION_DLG_UITHREADCMD,
        SELECT_THUMBNAIL_DLG_UITHREADCMD,
        HEIGHTFIELD_DIMENSION_DLG_UITHREADCMD,
        OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_UITHREADCMD,
        OPEN_MODAL_MODEL_PROPERTIES_UITHREADCMD,
        REFRESH_DIALOGS_UITHREADCMD,
        CALL_DIALOG_FUNCTION_UITHREADCMD,
        PLUS_CVU_CMD_UITHREADCMD,
        PLUS_HVUD_CMD_UITHREADCMD,
        TOGGLE_VISUALIZATION_UITHREADCMD,
        VISUALIZATION_OFF_UITHREADCMD,
        VISUALIZATION_ON_UITHREADCMD,
//        FLASH_STATUSBAR_UITHREADCMD,
        OPEN_HIERARCHY_UITHREADCMD,
        CLOSE_HIERARCHY_UITHREADCMD,

        MAIN_WINDOW_START_MWUITHREADCMD,
        MAIN_WINDOW_NON_EDIT_MODE_DLG_CLOSE_MWUITHREADCMD,
        MAIN_WINDOW_NON_EDIT_MODE_DLG_REOPEN_MWUITHREADCMD,
        MAIN_WINDOW_VIEW_SELECTOR_DLG_CLOSE_MWUITHREADCMD,
        MAIN_WINDOW_VIEW_SELECTOR_DLG_REOPEN_MWUITHREADCMD,
        MAIN_WINDOW_PAGE_SELECTOR_DLG_CLOSE_MWUITHREADCMD,
        MAIN_WINDOW_PAGE_SELECTOR_DLG_REOPEN_MWUITHREADCMD,
        MAIN_WINDOW_SET_FULLSCREEN_MWTHREADCMD,
        MAIN_WINDOW_ACTIVATE_MWUITHREADCMD,
        MAIN_WINDOW_CLOSE_DLG_MWUITHREADCMD,
        MAIN_WINDOW_OPEN_DLG_OR_BRING_TO_FRONT_MWUITHREADCMD,
        MAIN_WINDOW_END_MWUITHREADCMD,

        SERIAL_PORT_START_SPUITHREADCMD,
        SERIAL_PORT_ALT_OPEN_SPUITHREADCMD,
        SERIAL_PORT_ALT_CLOSE_SPUITHREADCMD,
        SERIAL_PORT_ALT_CHECK_SPUITHREADCMD,
        SERIAL_PORT_ALT_SEND_SPUITHREADCMD,
        SERIAL_PORT_ALT_RECEIVE_SPUITHREADCMD,
        SERIAL_PORT_END_SPUITHREADCMD,

        DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD,
        SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD,
        DISPLAY_MESH_DECIMATION_DIALOG_UITHREADCMD,
        DISPLAY_MESH_INSIDE_EXTRACTION_ITERATIONS_DIALOG_UITHREADCMD,

        DISPLAY_OR_HIDE_PROGRESS_DIALOG_UITHREADCMD,


        PLUGIN_START_PLUGUITHREADCMD,
        PLUGIN_LOAD_AND_START_PLUGUITHREADCMD,
        PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD,
        PLUGIN_END_PLUGUITHREADCMD,

        DESTROY_GL_TEXTURE_UITHREADCMD,
        DISPLAY_FILE_DLG_UITHREADCMD,
        DISPLAY_MSG_WITH_CHECKBOX_UITHREADCMD,
        DISPLAY_MSGBOX_UITHREADCMD,
        DETECT_VISION_SENSOR_ENTITY_UITHREADCMD,
        DISPLAY_SAVE_DLG_UITHREADCMD,
        DISPLAY_OPEN_DLG_UITHREADCMD,
        DISPLAY_OPEN_DLG_MULTIFILE_UITHREADCMD,
        DLG_INPUT_GET_FLOAT_UITHREADCMD,
        CREATE_GL_CONTEXT_FBO_TEXTURE_IF_NEEDED_UITHREADCMD,
        SET_FILEDIALOG_NATIVE_UITHREADCMD,
        SHOW_PRIMITIVE_SHAPE_DLG_UITHREADCMD,

        LOG_MSG_TO_STATUSBAR_UITHREADCMD,
        CLEAR_STATUSBAR_UITHREADCMD,
        NO_SIGNAL_SLOT_EXIT_UITHREADCMD,

        INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD,
        INSTANCE_WAS_JUST_CREATED_UITHREADCMD,
        INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD,
        INSTANCE_HAS_CHANGE_UITHREADCMD,
        INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD,
        NEW_SCENE_NAME_UITHREADCMD,
        SIMULATION_ABOUT_TO_START_UITHREADCMD,
        SIMULATION_JUST_ENDED_UITHREADCMD,
        EDIT_MODE_ABOUT_TO_START_UITHREADCMD,
        EDIT_MODE_JUST_ENDED_UITHREADCMD,
        COPY_TEXT_TO_CLIPBOARD_UITHREADCMD,
        INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD,

        MENUBAR_COLOR_UITHREADCMD,
     };

#ifndef SIM_WITH_QT
class CUiThread
{
#else
#include <QObject>
class CUiThread : public QObject
{
    Q_OBJECT
#endif
public:
    CUiThread();
    virtual ~CUiThread();

    bool executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    void showOrHideProgressBar(bool show,float pos=999.0f,const char* txt=nullptr);
    bool showOrHideEmergencyStop(bool show,const char* txt);

#ifndef SIM_WITH_QT
    void processGuiEventsUntilQuit_noSignalSlots();
#endif

private:
    int _frameId;
    int _lastFrameId;

#ifdef SIM_WITH_QT
signals:
    void _executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

private slots:
    void __executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
#else
    void __executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    VMutex _noSigSlotMutex;
    int _noSigSlot_cnter;
    SUIThreadCommand* _noSigSlot_cmdIn;
    SUIThreadCommand* _noSigSlot_cmdOut;
#endif

#ifdef SIM_WITH_GUI
public:
    int getLastFrameId();
    void setLastFrameId(int fid);
    void requestSceneRender_wait();
    bool messageBox_checkbox(void* parentWidget,const char* title,const char* message,const char* checkboxMessage,bool isWarning);
    void setFileDialogsNative(int n);
    std::string getOpenFileName(void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    bool getOpenFileNames(std::vector<std::string>& files,void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    std::string getSaveFileName(void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    std::string getOpenOrSaveFileName_api(int mode,const char* title,const char* startPath,const char* initName,const char* extName,const char* ext);
    bool showPrimitiveShapeDialog(int type,const C3Vector* optionalSizesIn,C3Vector& sizes,int subdiv[3],int& faces,int& sides,int& discSubdiv,bool& smooth,int& openEnds,bool& dynamic,bool& pure,bool& cone,float& density);
    unsigned short messageBox_informationSystemModal(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);
    unsigned short messageBox_information(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);
    unsigned short messageBox_question(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);
    unsigned short messageBox_warning(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);
    unsigned short messageBox_critical(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);
    bool dialogInputGetFloat(void* parentWidget,const char* title,const char* msg,float def,float minV,float maxV,int decimals,float* outFloat);

private:
    unsigned short _messageBox(int type,void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer);

signals:
    void _requestSceneRender_wait();

private slots:
    void __requestSceneRender_wait();
#endif
};
