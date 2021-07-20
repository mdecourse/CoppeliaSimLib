#include "confReaderAndWriter.h"
#include "simInternal.h"
#include "userSettings.h"
#include "global.h"
#include "threadPool_old.h"
#include "tt.h"
#include "easyLock.h"
#include "vVarious.h"
#include "app.h"
#include "simFlavor.h"
#ifdef SIM_WITH_GUI
    #include "vDialog.h"
#endif

#define USER_SETTINGS_FILENAME "usrset.txt"

#define _USR_ANTIALIASING "antialiasing"
#define _USR_DISPLAY_WORLD_REF "displayWorldRef"
#define _USR_USE_GLFINISH "useGlFinish"
#define _USR_USE_GLFINISH_VISION_SENSORS "useGlFinish_visionSensors"
#define _USR_OGL_COMPATIBILITY_TWEAK_1 "oglCompatibilityTweak1"

#define _USR_STEREO_DIST "stereoDist"
#define _USR_VSYNC "vsync"
#define _USR_DEBUG_OPENGL "debugOpenGl"
#define _USR_DISPLAY_BOUNDING_BOXES "displayBoundingBoxeWhenObjectSelected"
#define _USR_REMOVE_IDENTICAL_VERTICES "removeIdenticalVertices"
#define _USR_IDENTICAL_VERTICES_TOLERANCE "identicalVerticesTolerance"
#define _USR_REMOVE_IDENTICAL_TRIANGLES "removeIdenticalTriangles"
#define _USR_TRIANGLE_WINDING_CHECK "triangleWindingCheck"
#define _USR_PROCESSOR_CORE_AFFINITY "processorCoreAffinity"
#define _USR_DYNAMIC_ACTIVITY_RANGE "dynamicActivityRange"
#define _USR_FREE_SERVER_PORT_START "freeServerPortStart"
#define _USR_FREE_SERVER_PORT_RANGE "freeServerPortRange"
#define _USR_GUI_FONT_SIZE_WIN "guiFontSize_Win"
#define _USR_GUI_FONT_SIZE_MAC "guiFontSize_Mac"
#define _USR_GUI_FONT_SIZE_LINUX "guiFontSize_Linux"
#define _USR_TRANSLATION_STEP_SIZE "objectTranslationStepSize"
#define _USR_ROTATION_STEP_SIZE "objectRotationStepSize"
#define _USR_COMPRESS_FILES "compressFiles"
#define _USR_TRIANGLE_COUNT_IN_OBB "triCountInOBB"
#define _USR_APPROXIMATED_NORMALS "saveApproxNormals"
#define _USR_PACK_INDICES "packIndices"
#define _USR_UNDO_REDO_ENABLED "undoRedoEnabled"
#define _USR_UNDO_REDO_PARTIAL_WITH_CAMERAS "undoRedoOnlyPartialWithCameras"
#define _USR_UNDO_REDO_LEVEL_COUNT "undoRedoLevelCount"
#define _USR_RUN_CUSTOMIZATION_SCRIPTS "runCustomizationScripts"
#define _USR_TEST1 "test1"
#define _USR_ORDER_HIERARCHY_ALPHABETICALLY "orderHierarchyAlphabetically"
#define _USR_MAC_CHILD_DIALOG_TYPE "macChildDialogType"
#define _USR_DESKTOP_RECORDING_INDEX "desktopRecordingIndex"
#define _USR_DESKTOP_RECORDING_WIDTH "desktopRecordingWidth"
#define _USR_EXTERNAL_SCRIPT_EDITOR "externalScriptEditor"
#define _USR_XML_EXPORT_SPLIT_SIZE "xmlExportSplitSize"
#define _USR_XML_EXPORT_KNOWN_FORMATS "xmlExportKnownFormats"


#define _USR_IDLE_FPS "idleFps"
#define _USR_UNDO_REDO_MAX_BUFFER_SIZE "undoRedoMaxBufferSize"
#define _USR_ALWAYS_SHOW_CONSOLE "alwaysShowConsole"
#define _USR_VERBOSITY "verbosity"
#define _USR_STATUSBAR_VERBOSITY "statusbarVerbosity"
#define _USR_DIALOG_VERBOSITY "dialogVerbosity"
#define _USR_LOG_FILTER "logFilter"
#define _USR_UNDECORATED_STATUSBAR_MSGS "undecoratedStatusbarMessages"
#define _USR_CONSOLE_MSGS_TO_FILE "consoleMsgsToFile"
#define _USR_FORCE_BUG_FIX_REL_30002 "forceBugFix_rel30002"
#define _USR_STATUSBAR_INITIALLY_VISIBLE "statusbarInitiallyVisible"
#define _USR_MODELBROWSER_INITIALLY_VISIBLE "modelBrowserInitiallyVisible"
#define _USR_SCENEHIERARCHY_INITIALLY_VISIBLE "sceneHierarchyInitiallyVisible"
#define _USR_SCENEHIERARCHY_HIDDEN_DURING_SIMULATION "sceneHierarchyHiddenDuringSimulation"
#define _USR_AUTO_SAVE_DELAY "autoSaveDelay"
#define _USR_BULLET_SERIALIZATION_BUFFER "bulletSerializationBuffer"
#define _USR_TIME_FOR_UNDO_REDO_TOO_LONG_WARNING "timeInMsForUndoRedoTooLongWarning"
#define _USR_MIDDLE_MOUSE_BUTTON_SWITCHES_MODES "middleMouseButtonSwitchesModes"
#define _USR_NAVIGATION_BACKWARD_COMPATIBILITY_MODE "navigationBackwardCompatibility"
#define _USR_COLOR_ADJUST_BACK_COMPATIBILITY "colorAdjust_backCompatibility"
#define _USR_SPECIFIC_GPU_TWEAK "specificGpuTweak"
#define _USR_USE_ALTERNATE_SERIAL_PORT_ROUTINES "useAlternateSerialPortRoutines"
#define _USR_DISABLED_OPENGL_BASED_CUSTOM_UI "disableOpenGlBasedCustomUi"
#define _USR_SHOW_old_DLGS "showOldDlgs"
#define _USR_ENABLE_OLD_RENDERABLE "enableOldRenderableBehaviour"
#define _USR_BUGFIX1 "bugFix1"
#define _USR_COMPATIBILITYFIX1 "compatibilityFix1"
#define _USR_SUPPORT_old_THREADED_SCRIPTS "keepOldThreadedScripts"
#define _USR_SUPPORT_old_API_NOTATION "supportOldApiNotation"
#define _USR_ENABLE_old_MIRROR_OBJECTS "enableOldMirrorObjects"
#define _USR_ALLOW_old_EDU_RELEASE "allowOldEduRelease"
#define _USR_THREADED_SCRIPTS_GRACE_TIME "threadedScriptsStoppingGraceTime"

#define _USR_ABORT_SCRIPT_EXECUTION_BUTTON "abortScriptExecutionButton"
#define _USR_DARK_MODE "darkMode"
#define _USR_RENDERING_SURFACE_VERTICAL_SHIFT "renderingSurfaceVShift"
#define _USR_RENDERING_SURFACE_VERTICAL_RESIZE "renderingSurfaceVResize"
#define _USR_ADDITIONAL_LUA_PATH "additionalLuaPath"

#define _USR_DIRECTORY_FOR_SCENES "defaultDirectoryForScenes"
#define _USR_DIRECTORY_FOR_MODELS "defaultDirectoryForModels"
#define _USR_DIRECTORY_FOR_CAD "defaultDirectoryForCadFiles"
#define _USR_DIRECTORY_FOR_MISC "defaultDirectoryForMiscFiles"


#define _USR_OFFSCREEN_CONTEXT_TYPE "offscreenContextType"
#define _USR_FBO_TYPE "fboType"
#define _USR_FORCE_FBO_VIA_EXT "forceFboViaExt"
#define _USR_VBO_OPERATION "vboOperation"
#define _USR_VBO_PERSISTENCE_IN_MS "vboPersistenceInMs"
#define _USR_DESIRED_OPENGL_MAJOR "desiredOpenGlMajor"
#define _USR_DESIRED_OPENGL_MINOR "desiredOpenGlMinor"
#define _USR_HIGH_RES_DISPLAY "highResDisplay"
#define _USR_NO_EDGES_WHEN_MOUSE_DOWN "noEdgesWhenMouseDownInCameraView"
#define _USR_NO_TEXTURES_WHEN_MOUSE_DOWN "noTexturesWhenMouseDownInCameraView"
#define _USR_NO_CUSTOM_UIS_WHEN_MOUSE_DOWN "noCustomUisWhenMouseDownInCameraView"
#define _USR_HIERARCHY_REFRESH_CNT "hierarchyRefreshCnt"

#define _USR_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND "threadedChildScriptColor_background"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_SELECTION "threadedChildScriptColor_selection"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_COMMENT "threadedChildScriptColor_comment"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_NUMBER "threadedChildScriptColor_number"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_STRING "threadedChildScriptColor_string"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_CHARACTER "threadedChildScriptColor_character"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_OPERATOR "threadedChildScriptColor_operator"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR "threadedChildScriptColor_preprocessor"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER "threadedChildScriptColor_identifier"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_WORD "threadedChildScriptColor_word"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_WORD2 "threadedChildScriptColor_word2"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_WORD3 "threadedChildScriptColor_word3"
#define _USR_THREADED_CHILD_SCRIPT_COLOR_WORD4 "threadedChildScriptColor_word4"

#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND "nonThreadedChildScriptColor_background"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_SELECTION "nonThreadedChildScriptColor_selection"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_COMMENT "nonThreadedChildScriptColor_comment"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_NUMBER "nonThreadedChildScriptColor_number"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_STRING "nonThreadedChildScriptColor_string"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_CHARACTER "nonThreadedChildScriptColor_character"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_OPERATOR "nonThreadedChildScriptColor_operator"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR "nonThreadedChildScriptColor_preprocessor"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER "nonThreadedChildScriptColor_identifier"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD "nonThreadedChildScriptColor_word"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD2 "nonThreadedChildScriptColor_word2"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD3 "nonThreadedChildScriptColor_word3"
#define _USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD4 "nonThreadedChildScriptColor_word4"

#define _USR_MAIN_SCRIPT_COLOR_BACKGROUND "mainScriptColor_background"
#define _USR_MAIN_SCRIPT_COLOR_SELECTION "mainScriptColor_selection"
#define _USR_MAIN_SCRIPT_COLOR_COMMENT "mainScriptColor_comment"
#define _USR_MAIN_SCRIPT_COLOR_NUMBER "mainScriptColor_number"
#define _USR_MAIN_SCRIPT_COLOR_STRING "mainScriptColor_string"
#define _USR_MAIN_SCRIPT_COLOR_CHARACTER "mainScriptColor_character"
#define _USR_MAIN_SCRIPT_COLOR_OPERATOR "mainScriptColor_operator"
#define _USR_MAIN_SCRIPT_COLOR_PREPROCESSOR "mainScriptColor_preprocessor"
#define _USR_MAIN_SCRIPT_COLOR_IDENTIFIER "mainScriptColor_identifier"
#define _USR_MAIN_SCRIPT_COLOR_WORD "mainScriptColor_word"
#define _USR_MAIN_SCRIPT_COLOR_WORD2 "mainScriptColor_word2"
#define _USR_MAIN_SCRIPT_COLOR_WORD3 "mainScriptColor_word3"
#define _USR_MAIN_SCRIPT_COLOR_WORD4 "mainScriptColor_word4"

#define _USR_CUSTOMIZATION_SCRIPT_COLOR_BACKGROUND "customizationScriptColor_background"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_SELECTION "customizationScriptColor_selection"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_COMMENT "customizationScriptColor_comment"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_NUMBER "customizationScriptColor_number"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_STRING "customizationScriptColor_string"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_CHARACTER "customizationScriptColor_character"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_OPERATOR "customizationScriptColor_operator"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_PREPROCESSOR "customizationScriptColor_preprocessor"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_IDENTIFIER "customizationScriptColor_identifier"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_WORD "customizationScriptColor_word"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_WORD2 "customizationScriptColor_word2"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_WORD3 "customizationScriptColor_word3"
#define _USR_CUSTOMIZATION_SCRIPT_COLOR_WORD4 "customizationScriptColor_word4"

#define _USR_VISION_SENSORS_USE_GUI_WINDOWED "visionSensorsUseGuiThread_windowed"
#define _USR_VISION_SENSORS_USE_GUI_HEADLESS "visionSensorsUseGuiThread_headless"
#define _USR_FILE_DIALOGS_NATIVE "fileDialogs"
#define _USR_MOUSE_WHEEL_ZOOM_FACTOR "mouseWheelZoomFactor"

#define _USR_DO_NOT_WRITE_PERSISTENT_DATA "doNotWritePersistentData"
#define _USR_DO_NOT_SHOW_CRASH_RECOVERY_MESSAGE "doNotShowCrashRecoveryMessage"
#define _USR_DO_NOT_SHOW_UPDATE_CHECK_MESSAGE "doNotShowUpdateCheckMessage"
#define _USR_DO_NOT_SHOW_PROGRESS_BARS "doNotShowProgressBars"
#define _USR_DO_NOT_SHOW_ACKNOWLEDGMENT_MESSAGES "doNotShowAcknowledgmentMessages"
#define _USR_DO_NOT_SHOW_VIDEO_COMPRESSION_LIBRARY_LOAD_ERROR "doNotShowVideoCompressionLibraryLoadError"
#define _USR_SUPPRESS_STARTUP_DIALOG "suppressStartupDialogs"
#define _USR_SUPPRESS_XML_OVERWRITE_MSG "suppressXmlOverwriteMsg"

#define _USR_SCRIPT_EDITOR_FONT "scriptEditorFont"
#define _USR_SCRIPT_EDITOR_FONT_SIZE "scriptEditorFontSize"
#define _USR_SCRIPT_EDITOR_BOLDFONT "scriptEditorBoldFont"

#define _USR_CONNECTION_ADDRESS "conParam1"
#define _USR_CONNECTION_PORT "conParam2"

#define _USR_FLOAT_LICENSE_ENABLED "floatingLicenseEnabled"
#define _USR_FLOAT_LICENSE_SERVER_ADDRESS "floatingLicenseServer"
#define _USR_FLOAT_LICENSE_SERVER_PORT "floatingLicensePort"
#define _USR_KEEP_DONGLE_OPEN "keepDongleOpen"
#define _USR_XR_TEST "xrTest"

CUserSettings::CUserSettings()
{
    // Debugging section:
    // *****************************
    alwaysShowConsole=false;
    _overrideConsoleVerbosity="default";
    _overrideStatusbarVerbosity="default";
    _overrideDialogVerbosity="default";
    _consoleLogFilter="";
    undecoratedStatusbarMessages=false;

    // Rendering section:
    // *****************************
    _idleFps=8;
    desiredOpenGlMajor=-1; // default
    desiredOpenGlMinor=-1; // default
    offscreenContextType=-1; // default type
    fboType=-1; // default type
    forceFboViaExt=false; // default
    vboOperation=-1; // default
    vboPersistenceInMs=5000; // default
    oglCompatibilityTweak1=false;
    visionSensorsUseGuiThread_windowed=-1; // default
    visionSensorsUseGuiThread_headless=-1; // default
    useGlFinish=false;
    useGlFinish_visionSensors=false;
    vsync=0;
    debugOpenGl=false;
    stereoDist=0.0f; // default, no stereo!


    // Visual section:
    // *****************************
    darkMode=false;
    renderingSurfaceVShift=0;
    renderingSurfaceVResize=0;
    scriptEditorFont=""; // default
    scriptEditorFontSize=-1; // default
    scriptEditorBoldFont=false;

    displayWorldReference=true;
    antiAliasing=false;
    displayBoundingBoxeWhenObjectSelected=true;
    guiFontSize_Win=13; // 11-14 ok
    guiFontSize_Mac=-1; // 10-13 ok
    guiFontSize_Linux=13; // 11-14 ok, default is quite large
    statusbarInitiallyVisible=CSimFlavor::getBoolVal(10);
    modelBrowserInitiallyVisible=true;
    sceneHierarchyInitiallyVisible=true;
    sceneHierarchyHiddenDuringSimulation=false;

    highResDisplay=-1;
    noEdgesWhenMouseDownInCameraView=false;
    noTexturesWhenMouseDownInCameraView=false;
    noCustomUisWhenMouseDownInCameraView=true;
    hierarchyRefreshCnt=3;

    _setIntVector3(threadedChildScriptColor_background,180,190,190);
    _setIntVector3(threadedChildScriptColor_selection,128,128,255);
    _setIntVector3(threadedChildScriptColor_comment,0,140,0);
    _setIntVector3(threadedChildScriptColor_number,220,0,220);
    _setIntVector3(threadedChildScriptColor_string,255,255,0);
    _setIntVector3(threadedChildScriptColor_character,255,255,0);
    _setIntVector3(threadedChildScriptColor_operator,0,0,0);
    _setIntVector3(threadedChildScriptColor_preprocessor,0,128,128);
    _setIntVector3(threadedChildScriptColor_identifier,64,64,64);
    _setIntVector3(threadedChildScriptColor_word,0,0,255);
    _setIntVector3(threadedChildScriptColor_word2,152,0,0);
    _setIntVector3(threadedChildScriptColor_word3,220,80,20);
    _setIntVector3(threadedChildScriptColor_word4,152,64,0);

    _setIntVector3(nonThreadedChildScriptColor_background,190,190,190);
    _setIntVector3(nonThreadedChildScriptColor_selection,128,128,255);
    _setIntVector3(nonThreadedChildScriptColor_comment,0,140,0);
    _setIntVector3(nonThreadedChildScriptColor_number,220,0,220);
    _setIntVector3(nonThreadedChildScriptColor_string,255,255,0);
    _setIntVector3(nonThreadedChildScriptColor_character,255,255,0);
    _setIntVector3(nonThreadedChildScriptColor_operator,0,0,0);
    _setIntVector3(nonThreadedChildScriptColor_preprocessor,0,128,128);
    _setIntVector3(nonThreadedChildScriptColor_identifier,64,64,64);
    _setIntVector3(nonThreadedChildScriptColor_word,0,0,255);
    _setIntVector3(nonThreadedChildScriptColor_word2,152,0,0);
    _setIntVector3(nonThreadedChildScriptColor_word3,220,80,20);
    _setIntVector3(nonThreadedChildScriptColor_word4,152,64,0);

    _setIntVector3(mainScriptColor_background,190,175,175);
    _setIntVector3(mainScriptColor_selection,128,128,255);
    _setIntVector3(mainScriptColor_comment,0,140,0);
    _setIntVector3(mainScriptColor_number,220,0,220);
    _setIntVector3(mainScriptColor_string,255,255,0);
    _setIntVector3(mainScriptColor_character,255,255,0);
    _setIntVector3(mainScriptColor_operator,0,0,0);
    _setIntVector3(mainScriptColor_preprocessor,0,128,128);
    _setIntVector3(mainScriptColor_identifier,64,64,64);
    _setIntVector3(mainScriptColor_word,0,0,255);
    _setIntVector3(mainScriptColor_word2,152,0,0);
    _setIntVector3(mainScriptColor_word3,220,80,20);
    _setIntVector3(mainScriptColor_word4,152,64,0);

    _setIntVector3(customizationScriptColor_background,150,150,150);
    _setIntVector3(customizationScriptColor_selection,128,128,255);
    _setIntVector3(customizationScriptColor_comment,0,120,0);
    _setIntVector3(customizationScriptColor_number,220,0,220);
    _setIntVector3(customizationScriptColor_string,205,205,0);
    _setIntVector3(customizationScriptColor_character,205,205,0);
    _setIntVector3(customizationScriptColor_operator,0,0,0);
    _setIntVector3(customizationScriptColor_preprocessor,0,128,128);
    _setIntVector3(customizationScriptColor_identifier,70,70,70);
    _setIntVector3(customizationScriptColor_word,0,0,255);
    _setIntVector3(customizationScriptColor_word2,152,0,0);
    _setIntVector3(customizationScriptColor_word3,220,80,20);
    _setIntVector3(customizationScriptColor_word4,152,64,0);


    // Directories section:
    // *****************************
    defaultDirectoryForScenes="";
    defaultDirectoryForModels="";
    defaultDirectoryForCadFiles="";
    defaultDirectoryForMiscFiles="";


    // Serialization section:
    // *****************************
    autoSaveDelay=2; // 2 minutes for an auto-save by default. set to 0 for no auto-save
    doNotWritePersistentData=false;
    compressFiles=true;
    saveApproxNormals=true;
    packIndices=true;
    fileDialogs=-1; // default
    bulletSerializationBuffer=10000000;



    // Undo/redo section:
    // *****************************
    _undoRedoEnabled=true;
    _undoRedoOnlyPartialWithCameras=true;
    undoRedoLevelCount=16;
    undoRedoMaxBufferSize=80000000;
    timeInMsForUndoRedoTooLongWarning=200;


    // Messaging section:
    // *****************************
    doNotShowCrashRecoveryMessage=false;
    doNotShowUpdateCheckMessage=false;
    doNotShowProgressBars=false;
    doNotShowAcknowledgmentMessages=false;
    doNotShowVideoCompressionLibraryLoadError=false;
    suppressStartupDialogs=false;
    suppressXmlOverwriteMsg=false;


    // Compatibility section:
    // *****************************
    middleMouseButtonSwitchesModes=false; // set to false on 28/9/2014
    navigationBackwardCompatibility=false;
    colorAdjust_backCompatibility=1.0f; // default
    specificGpuTweak=false; // default
    useAlternateSerialPortRoutines=false;
    disableOpenGlBasedCustomUi=false;
    showOldDlgs=false;
    enableOldRenderableBehaviour=false;
    keepOldThreadedScripts=false;
    _supportOldApiNotation=true;
    enableOldMirrorObjects=false;
    allowOldEduRelease=-1;
    threadedScriptsStoppingGraceTime=0;
    bugFix1=1000;
    compatibilityFix1=false;


    // Various section:
    // *****************************
    mouseWheelZoomFactor=1.0f; // default
    dynamicActivityRange=1000.0f;
    _translationStepSize=0.025f;
    _rotationStepSize=5.0f*degToRad_f;
    freeServerPortStart=20000;
    _nextfreeServerPortToUse=freeServerPortStart;
    freeServerPortRange=2000;
    _abortScriptExecutionButton=3;
    triCountInOBB=8; // gave best results in 2009/07/21
    identicalVerticesCheck=true;
    identicalVerticesTolerance=0.0001f;
    identicalTrianglesCheck=true;
    identicalTrianglesWindingCheck=true;
    runCustomizationScripts=true;
    test1=false;
    orderHierarchyAlphabetically=false;
    macChildDialogType=-1; // default
    additionalLuaPath="";
    desktopRecordingIndex=0;
    desktopRecordingWidth=-1;
    externalScriptEditor="";
    xmlExportSplitSize=0;
    xmlExportKnownFormats=true;

    forceBugFix_rel30002=false;

    floatingLicenseEnabled=false;
    floatingLicenseServer="127.0.0.1";
    floatingLicensePort=20249;
    keepDongleOpen=false;
    xrTest=0;

    loadUserSettings();
}

CUserSettings::~CUserSettings()
{
}

void CUserSettings::setTranslationStepSize(float s)
{
    float sc=1.0f;
    if ((s>=0.0075f)&&(s<0.075f))
        sc=10.0f;
    if (s>=0.075f)
        sc=100.0f;
    if (s<0.0015f*sc)
        s=0.001f*sc;
    else
    {
        if (s<0.00375f*sc)
        {
            if (sc<2.0f)
                s=0.002f*sc;
            else
                s=0.0025f*sc;
        }
        else
            s=0.005f*sc;
    }
    _translationStepSize=s;
}
float CUserSettings::getTranslationStepSize()
{
    return(_translationStepSize);
}

void CUserSettings::setRotationStepSize(float s)
{
    if (s<1.5f*degToRad_f)
        s=1.0f*degToRad_f;
    else
    {
        if (s<3.5f*degToRad_f)
            s=2.0f*degToRad_f;
        else
        {
            if (s<7.5f*degToRad_f)
                s=5.0f*degToRad_f;
            else
            {
                if (s<12.5f*degToRad_f)
                    s=10.0f*degToRad_f;
                else
                {
                    if (s<22.5f*degToRad_f)
                        s=15.0f*degToRad_f;
                    else
                    {
                        if (s<37.5f*degToRad_f)
                            s=30.0f*degToRad_f;
                        else
                            s=45.0f*degToRad_f;
                    }
                }
            }
        }
    }
    _rotationStepSize=s;
}

float CUserSettings::getRotationStepSize()
{
    return(_rotationStepSize);
}

void CUserSettings::setUndoRedoEnabled(bool isEnabled)
{
    _undoRedoEnabled=isEnabled;
    if (App::currentWorld->undoBufferContainer!=nullptr)
        App::currentWorld->undoBufferContainer->emptySceneProcedure();
}

bool CUserSettings::getUndoRedoEnabled()
{
    return(_undoRedoEnabled);
}

bool CUserSettings::getUndoRedoOnlyPartialWithCameras()
{
    return(_undoRedoOnlyPartialWithCameras);
}

int CUserSettings::getIdleFps()
{
    return(_idleFps);
}

void CUserSettings::setIdleFps(int fps)
{
    _idleFps=tt::getLimitedInt(0,25,fps);
}

int CUserSettings::getAbortScriptExecutionTiming()
{
    if (!CSimFlavor::getBoolVal(11))
        return(0);
    return(_abortScriptExecutionButton);
}

int CUserSettings::getNextFreeServerPortToUse()
{
    return(_nextfreeServerPortToUse);
}

void CUserSettings::setNextFreeServerPortToUse(int p)
{
    _nextfreeServerPortToUse=p;
}

bool CUserSettings::getSupportOldApiNotation()
{
    static int support=0;
    if (support==0)
    {
        for (int i=0;i<9;i++)
        {
            std::string s(App::getApplicationArgument(i));
            if (s.compare("NO_old_API_SUPPORT")==0)
            {
                support=-1;
                break;
            }
        }
        if (support==0)
            support=1;
    }
    return(_supportOldApiNotation&&(support==1));
}

void CUserSettings::_setIntVector3(int v[3],int a,int b,int c)
{
    v[0]=a;
    v[1]=b;
    v[2]=c;
}

void CUserSettings::saveUserSettings()
{
    CConfReaderAndWriter c;

    c.addRandomLine("// Debugging");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_ALWAYS_SHOW_CONSOLE,alwaysShowConsole,"");
    c.addString(_USR_VERBOSITY,_overrideConsoleVerbosity,"to override console verbosity setting, use any of: default (do not override), none, errors, warnings, loadinfos, scripterrors, scriptwarnings, msgs, infos, debug, trace, tracelua or traceall");
    c.addString(_USR_STATUSBAR_VERBOSITY,_overrideStatusbarVerbosity,"to override statusbar verbosity setting, use any of: default (do not override), none, errors, warnings, loadinfos, scripterrors, scriptwarnings, msgs, infos, debug, trace, tracelua or traceall");
    c.addString(_USR_LOG_FILTER,_consoleLogFilter,"leave empty for no filter. Filter format: txta1&txta2&...&txtaN|txtb1&txtb2&...&txtbN|...");
    c.addString(_USR_DIALOG_VERBOSITY,_overrideDialogVerbosity,"to override dialog verbosity setting, use any of: default (do not override), none, errors, warnings, questions or infos");
    c.addBoolean(_USR_UNDECORATED_STATUSBAR_MSGS,undecoratedStatusbarMessages,"");
    c.addBoolean(_USR_CONSOLE_MSGS_TO_FILE,App::getConsoleMsgToFile(),"if true, console messages are sent to debugLog.txt");
    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Rendering");
    c.addRandomLine("// =================================================");
    c.addInteger(_USR_IDLE_FPS,_idleFps,"");
    c.addInteger(_USR_DESIRED_OPENGL_MAJOR,desiredOpenGlMajor,"recommended to keep -1.");
    c.addInteger(_USR_DESIRED_OPENGL_MINOR,desiredOpenGlMinor,"recommended to keep -1.");
    c.addInteger(_USR_OFFSCREEN_CONTEXT_TYPE,offscreenContextType,"recommended to keep -1 (-1=default, 0=Qt offscreen, 1=QGLWidget/QOpenGLWidget visible, 2=QGLWidget/QOpenGLWidget invisible).");
    c.addInteger(_USR_FBO_TYPE,fboType,"recommended to keep -1 (-1=default, 0=native, 1=QOpenGLFramebufferObject).");
    c.addBoolean(_USR_FORCE_FBO_VIA_EXT,forceFboViaExt,"recommended to keep false.");
    c.addInteger(_USR_VBO_OPERATION,vboOperation,"recommended to keep -1 (-1=default, 0=always off, 1=on when available).");
    c.addInteger(_USR_VBO_PERSISTENCE_IN_MS,vboPersistenceInMs,"recommended to keep 5000.");
    c.addBoolean(_USR_OGL_COMPATIBILITY_TWEAK_1,oglCompatibilityTweak1,"recommended to keep false since it causes small memory leaks.");
    c.addInteger(_USR_VISION_SENSORS_USE_GUI_WINDOWED,visionSensorsUseGuiThread_windowed,"recommended to keep -1 (-1=default, 0=GUI when not otherwise possible, 1=always GUI).");
    c.addInteger(_USR_VISION_SENSORS_USE_GUI_HEADLESS,visionSensorsUseGuiThread_headless,"recommended to keep -1 (-1=default, 0=GUI when not otherwise possible, 1=always GUI).");
    c.addBoolean(_USR_USE_GLFINISH,useGlFinish,"recommended to keep false. Graphic card dependent.");
    c.addBoolean(_USR_USE_GLFINISH_VISION_SENSORS,useGlFinish_visionSensors,"recommended to keep false. Graphic card dependent.");
    c.addInteger(_USR_VSYNC,vsync,"recommended to keep at 0. Graphic card dependent.");
    c.addBoolean(_USR_DEBUG_OPENGL,debugOpenGl,"");
    c.addFloat(_USR_STEREO_DIST,stereoDist,"0=no stereo, otherwise the intra occular distance (0.0635 for the human eyes).");
    c.addInteger(_USR_HIGH_RES_DISPLAY,highResDisplay,"-1=none, 2=enabled, 1=special.");
    c.addBoolean(_USR_NO_EDGES_WHEN_MOUSE_DOWN,noEdgesWhenMouseDownInCameraView,"if true, rendering is faster during mouse/view interaction");
    c.addBoolean(_USR_NO_TEXTURES_WHEN_MOUSE_DOWN,noTexturesWhenMouseDownInCameraView,"if true, rendering is faster during mouse/view interaction");
    c.addBoolean(_USR_NO_CUSTOM_UIS_WHEN_MOUSE_DOWN,noCustomUisWhenMouseDownInCameraView,"if true, rendering is faster during mouse/view interaction");
    c.addInteger(_USR_HIERARCHY_REFRESH_CNT,hierarchyRefreshCnt,"");


    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Visual");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_DARK_MODE,darkMode,"");
    c.addInteger(_USR_RENDERING_SURFACE_VERTICAL_SHIFT,renderingSurfaceVShift,"");
    c.addInteger(_USR_RENDERING_SURFACE_VERTICAL_RESIZE,renderingSurfaceVResize,"");
    c.addBoolean(_USR_DISPLAY_WORLD_REF,displayWorldReference,"");
    c.addBoolean(_USR_ANTIALIASING,antiAliasing,"");
    c.addBoolean(_USR_DISPLAY_BOUNDING_BOXES,displayBoundingBoxeWhenObjectSelected,"");
    c.addInteger(_USR_GUI_FONT_SIZE_WIN,guiFontSize_Win,"-1=default");
    c.addInteger(_USR_GUI_FONT_SIZE_MAC,guiFontSize_Mac,"-1=default");
    c.addInteger(_USR_GUI_FONT_SIZE_LINUX,guiFontSize_Linux,"-1=default");
    c.addBoolean(_USR_STATUSBAR_INITIALLY_VISIBLE,statusbarInitiallyVisible,"");
    c.addBoolean(_USR_MODELBROWSER_INITIALLY_VISIBLE,modelBrowserInitiallyVisible,"");
    c.addBoolean(_USR_SCENEHIERARCHY_INITIALLY_VISIBLE,sceneHierarchyInitiallyVisible,"");
    c.addBoolean(_USR_SCENEHIERARCHY_HIDDEN_DURING_SIMULATION,sceneHierarchyHiddenDuringSimulation,"");

    c.addString(_USR_SCRIPT_EDITOR_FONT,scriptEditorFont,"empty=default.");
    c.addBoolean(_USR_SCRIPT_EDITOR_BOLDFONT,scriptEditorBoldFont,"");
    c.addInteger(_USR_SCRIPT_EDITOR_FONT_SIZE,scriptEditorFontSize,"-1=default.");

    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_BACKGROUND,mainScriptColor_background,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_SELECTION,mainScriptColor_selection,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_COMMENT,mainScriptColor_comment,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_NUMBER,mainScriptColor_number,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_STRING,mainScriptColor_string,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_CHARACTER,mainScriptColor_character,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_OPERATOR,mainScriptColor_operator,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_PREPROCESSOR,mainScriptColor_preprocessor,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_IDENTIFIER,mainScriptColor_identifier,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD,mainScriptColor_word,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD2,mainScriptColor_word2,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD3,mainScriptColor_word3,"");
    c.addIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD4,mainScriptColor_word4,"");

    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND,nonThreadedChildScriptColor_background,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_SELECTION,nonThreadedChildScriptColor_selection,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_COMMENT,nonThreadedChildScriptColor_comment,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_NUMBER,nonThreadedChildScriptColor_number,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_STRING,nonThreadedChildScriptColor_string,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_CHARACTER,nonThreadedChildScriptColor_character,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_OPERATOR,nonThreadedChildScriptColor_operator,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR,nonThreadedChildScriptColor_preprocessor,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER,nonThreadedChildScriptColor_identifier,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD,nonThreadedChildScriptColor_word,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD2,nonThreadedChildScriptColor_word2,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD3,nonThreadedChildScriptColor_word3,"");
    c.addIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD4,nonThreadedChildScriptColor_word4,"");

    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND,threadedChildScriptColor_background,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_SELECTION,threadedChildScriptColor_selection,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_COMMENT,threadedChildScriptColor_comment,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_NUMBER,threadedChildScriptColor_number,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_STRING,threadedChildScriptColor_string,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_CHARACTER,threadedChildScriptColor_character,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_OPERATOR,threadedChildScriptColor_operator,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR,threadedChildScriptColor_preprocessor,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER,threadedChildScriptColor_identifier,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD,threadedChildScriptColor_word,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD2,threadedChildScriptColor_word2,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD3,threadedChildScriptColor_word3,"");
    c.addIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD4,threadedChildScriptColor_word4,"");

    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_BACKGROUND,customizationScriptColor_background,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_SELECTION,customizationScriptColor_selection,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_COMMENT,customizationScriptColor_comment,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_NUMBER,customizationScriptColor_number,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_STRING,customizationScriptColor_string,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_CHARACTER,customizationScriptColor_character,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_OPERATOR,customizationScriptColor_operator,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_PREPROCESSOR,customizationScriptColor_preprocessor,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_IDENTIFIER,customizationScriptColor_identifier,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD,customizationScriptColor_word,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD2,customizationScriptColor_word2,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD3,customizationScriptColor_word3,"");
    c.addIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD4,customizationScriptColor_word4,"");

    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Directories");
    c.addRandomLine("// =================================================");
    c.addString(_USR_DIRECTORY_FOR_SCENES,defaultDirectoryForScenes,"absolute path, e.g. d:/myScenes (or leave empty for default path)");
    c.addString(_USR_DIRECTORY_FOR_MODELS,defaultDirectoryForModels,"absolute path, e.g. d:/myModels (or leave empty for default path)");
    c.addString(_USR_DIRECTORY_FOR_CAD,defaultDirectoryForCadFiles,"absolute path, e.g. d:/myCadFiles (or leave empty for default path)");
    c.addString(_USR_DIRECTORY_FOR_MISC,defaultDirectoryForMiscFiles,"absolute path, e.g. d:/myMiscFiles (or leave empty for default path)");


    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Serialization");
    c.addRandomLine("// =================================================");
    c.addInteger(_USR_AUTO_SAVE_DELAY,autoSaveDelay,"in minutes. 0 to disable.");
    c.addBoolean(_USR_DO_NOT_WRITE_PERSISTENT_DATA,doNotWritePersistentData,"");
    c.addBoolean(_USR_COMPRESS_FILES,compressFiles,"");
    c.addBoolean(_USR_APPROXIMATED_NORMALS,saveApproxNormals,"");
    c.addBoolean(_USR_PACK_INDICES,packIndices,"");
    c.addInteger(_USR_FILE_DIALOGS_NATIVE,fileDialogs,"recommended to keep -1 (-1=default, 0=native dialogs, 1=Qt dialogs).");
    c.addInteger(_USR_BULLET_SERIALIZATION_BUFFER,bulletSerializationBuffer,"");


    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Undo/Redo");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_UNDO_REDO_ENABLED,_undoRedoEnabled,"");
    c.addBoolean(_USR_UNDO_REDO_PARTIAL_WITH_CAMERAS,_undoRedoOnlyPartialWithCameras,"");
    c.addInteger(_USR_UNDO_REDO_LEVEL_COUNT,undoRedoLevelCount,"");
    c.addInteger(_USR_UNDO_REDO_MAX_BUFFER_SIZE,undoRedoMaxBufferSize,"");
    c.addInteger(_USR_TIME_FOR_UNDO_REDO_TOO_LONG_WARNING,timeInMsForUndoRedoTooLongWarning,"");
    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Messaging");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_DO_NOT_SHOW_CRASH_RECOVERY_MESSAGE,doNotShowCrashRecoveryMessage,"");
    c.addBoolean(_USR_DO_NOT_SHOW_UPDATE_CHECK_MESSAGE,doNotShowUpdateCheckMessage,"");
    c.addBoolean(_USR_DO_NOT_SHOW_PROGRESS_BARS,doNotShowProgressBars,"");
    c.addBoolean(_USR_DO_NOT_SHOW_ACKNOWLEDGMENT_MESSAGES,doNotShowAcknowledgmentMessages,"");
    c.addBoolean(_USR_DO_NOT_SHOW_VIDEO_COMPRESSION_LIBRARY_LOAD_ERROR,doNotShowVideoCompressionLibraryLoadError,"");
    c.addBoolean(_USR_SUPPRESS_STARTUP_DIALOG,suppressStartupDialogs,"");
    c.addBoolean(_USR_SUPPRESS_XML_OVERWRITE_MSG,suppressXmlOverwriteMsg,"");



    c.addRandomLine("");
    c.addRandomLine("");


    c.addRandomLine("// Compatibility");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_MIDDLE_MOUSE_BUTTON_SWITCHES_MODES,middleMouseButtonSwitchesModes,"has only an effect if navigationBackwardCompatibility is true.");
    c.addBoolean(_USR_NAVIGATION_BACKWARD_COMPATIBILITY_MODE,navigationBackwardCompatibility,"recommended to keep false.");
    c.addFloat(_USR_COLOR_ADJUST_BACK_COMPATIBILITY,colorAdjust_backCompatibility,"recommended to keep 1.0");
    c.addBoolean(_USR_SPECIFIC_GPU_TWEAK,specificGpuTweak,"");
    c.addBoolean(_USR_USE_ALTERNATE_SERIAL_PORT_ROUTINES,useAlternateSerialPortRoutines,"");
    c.addBoolean(_USR_DISABLED_OPENGL_BASED_CUSTOM_UI,disableOpenGlBasedCustomUi,"");
    c.addBoolean(_USR_SHOW_old_DLGS,showOldDlgs,"");
    c.addBoolean(_USR_ENABLE_OLD_RENDERABLE,enableOldRenderableBehaviour,"");
    c.addBoolean(_USR_SUPPORT_old_THREADED_SCRIPTS,keepOldThreadedScripts,"");
    c.addBoolean(_USR_ENABLE_old_MIRROR_OBJECTS,enableOldMirrorObjects,"");
    c.addInteger(_USR_THREADED_SCRIPTS_GRACE_TIME,threadedScriptsStoppingGraceTime,"");




    c.addRandomLine("");
    c.addRandomLine("");

    c.addRandomLine("// Various");
    c.addRandomLine("// =================================================");
    c.addFloat(_USR_MOUSE_WHEEL_ZOOM_FACTOR,mouseWheelZoomFactor,"");
    c.addFloat(_USR_DYNAMIC_ACTIVITY_RANGE,dynamicActivityRange,"");
    c.addFloat(_USR_TRANSLATION_STEP_SIZE,_translationStepSize,"");
    c.addFloat(_USR_ROTATION_STEP_SIZE,_rotationStepSize*radToDeg_f,"");
    c.addInteger(_USR_PROCESSOR_CORE_AFFINITY,CThreadPool_old::getProcessorCoreAffinity(),"recommended to keep 0 (-1:os default, 0:all threads on same core, m: affinity mask (bit1=core1, bit2=core2, etc.))");
    c.addInteger(_USR_FREE_SERVER_PORT_START,freeServerPortStart,"");
    c.addInteger(_USR_FREE_SERVER_PORT_RANGE,freeServerPortRange,"");
    c.addInteger(_USR_ABORT_SCRIPT_EXECUTION_BUTTON,_abortScriptExecutionButton,"in seconds. Zero to disable.");
    c.addInteger(_USR_TRIANGLE_COUNT_IN_OBB,triCountInOBB,"");
    c.addBoolean(_USR_REMOVE_IDENTICAL_VERTICES,identicalVerticesCheck,"");
    c.addFloat(_USR_IDENTICAL_VERTICES_TOLERANCE,identicalVerticesTolerance,"");
    c.addBoolean(_USR_REMOVE_IDENTICAL_TRIANGLES,identicalTrianglesCheck,"");
    c.addBoolean(_USR_TRIANGLE_WINDING_CHECK,identicalTrianglesWindingCheck,"");
    c.addBoolean(_USR_RUN_CUSTOMIZATION_SCRIPTS,runCustomizationScripts,"");
    c.addBoolean(_USR_TEST1,test1,"recommended to keep false.");
    c.addBoolean(_USR_ORDER_HIERARCHY_ALPHABETICALLY,orderHierarchyAlphabetically,"");
    c.addInteger(_USR_MAC_CHILD_DIALOG_TYPE,macChildDialogType,"-1=default.");
    c.addString(_USR_ADDITIONAL_LUA_PATH,additionalLuaPath,"e.g. d:/myLuaRoutines");
    c.addInteger(_USR_DESKTOP_RECORDING_INDEX,desktopRecordingIndex,"");
    c.addInteger(_USR_DESKTOP_RECORDING_WIDTH,desktopRecordingWidth,"-1=default.");
    c.addString(_USR_EXTERNAL_SCRIPT_EDITOR,externalScriptEditor,"");
    c.addInteger(_USR_XML_EXPORT_SPLIT_SIZE,xmlExportSplitSize,"0=generate a single file.");
    c.addBoolean(_USR_XML_EXPORT_KNOWN_FORMATS,xmlExportKnownFormats,"true=if several files are generated, mesh and image files are saved under known formats.");

    c.addRandomLine("");
    c.addRandomLine("");

    c.addRandomLine("// Floating license / dongle license");
    c.addRandomLine("// =================================================");
    c.addBoolean(_USR_FLOAT_LICENSE_ENABLED,floatingLicenseEnabled,"");
    c.addString(_USR_FLOAT_LICENSE_SERVER_ADDRESS,floatingLicenseServer,"");
    c.addInteger(_USR_FLOAT_LICENSE_SERVER_PORT,floatingLicensePort,"");
    c.addBoolean(_USR_KEEP_DONGLE_OPEN,keepDongleOpen,"");
    // c.addInteger(_USR_XR_TEST,xrTest,"");

    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME);
    c.writeConfiguration(filenameAndPath.c_str());
}

bool CUserSettings::getBooleanFromFileDirectly(const char* varName,bool& varValue)
{ // static function
    bool result=false;
    CConfReaderAndWriter c;
    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME); // The CFolderSystem object might not yet be set-up
    if (c.readConfiguration(filenameAndPath.c_str()))
        result=c.getBoolean(varName,varValue);
    return(result);
}

bool CUserSettings::getIntegerFromFileDirectly(const char* varName,int& varValue)
{ // static function
    bool result=false;
    CConfReaderAndWriter c;
    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME); // The CFolderSystem object might not yet be set-up
    if (c.readConfiguration(filenameAndPath.c_str()))
        result=c.getInteger(varName,varValue);
    return(result);
}

bool CUserSettings::getFloatFromFileDirectly(const char* varName,float& varValue)
{ // static function
    bool result=false;
    CConfReaderAndWriter c;
    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME); // The CFolderSystem object might not yet be set-up
    if (c.readConfiguration(filenameAndPath.c_str()))
        result=c.getFloat(varName,varValue);
    return(result);
}

bool CUserSettings::getStringFromFileDirectly(const char* varName,std::string& varValue)
{ // static function
    bool result=false;
    CConfReaderAndWriter c;
    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME); // The CFolderSystem object might not yet be set-up
    if (c.readConfiguration(filenameAndPath.c_str()))
        result=c.getString(varName,varValue);
    return(result);
}


void CUserSettings::loadUserSettings()
{
    CConfReaderAndWriter c;

    // The CFolderSystem object might not yet be set-up
    std::string filenameAndPath(VVarious::getModulePath()+"/"+SIM_SYSTEM_DIRECTORY_NAME+"/"+USER_SETTINGS_FILENAME);
    // Following call might fail.
    if (!c.readConfiguration(filenameAndPath.c_str()))
        saveUserSettings();

    // Debugging section:
    // *****************************
    c.getBoolean(_USR_ALWAYS_SHOW_CONSOLE,alwaysShowConsole);
    c.getString(_USR_VERBOSITY,_overrideConsoleVerbosity);
    if (_overrideConsoleVerbosity.compare("default")!=0)
    {
        int l=App::getVerbosityLevelFromString(_overrideConsoleVerbosity.c_str());
        if (l>=sim_verbosity_none)
        {
            App::setConsoleVerbosity(l);
            App::logMsg(sim_verbosity_warnings,"console verbosity overridden to '%s' via system/usrset.txt.",_overrideConsoleVerbosity.c_str());
        }
        else
            App::logMsg(sim_verbosity_errors,"unrecognized verbosity value in system/usrset.txt: %s.",_overrideConsoleVerbosity.c_str());
    }
    c.getString(_USR_STATUSBAR_VERBOSITY,_overrideStatusbarVerbosity);
    if (_overrideStatusbarVerbosity.compare("default")!=0)
    {
        int l=App::getVerbosityLevelFromString(_overrideStatusbarVerbosity.c_str());
        if (l>=sim_verbosity_none)
        {
            App::setStatusbarVerbosity(l);
            App::logMsg(sim_verbosity_warnings,"statusbar verbosity overridden to '%s' via system/usrset.txt.",_overrideStatusbarVerbosity.c_str());
        }
        else
            App::logMsg(sim_verbosity_errors,"unrecognized verbosity value in system/usrset.txt: %s.",_overrideStatusbarVerbosity.c_str());
    }
    c.getString(_USR_LOG_FILTER,_consoleLogFilter);
    App::setConsoleLogFilter(_consoleLogFilter.c_str());

    c.getString(_USR_DIALOG_VERBOSITY,_overrideDialogVerbosity);
    if (_overrideDialogVerbosity.compare("default")!=0)
    {
        int l=App::getVerbosityLevelFromString(_overrideDialogVerbosity.c_str());
        if (l>=sim_verbosity_none)
        {
            App::setDlgVerbosity(l);
            App::logMsg(sim_verbosity_warnings,"dialog verbosity overridden to '%s' via system/usrset.txt.",_overrideDialogVerbosity.c_str());
        }
        else
            App::logMsg(sim_verbosity_errors,"unrecognized verbosity value in system/usrset.txt: %s.",_overrideDialogVerbosity.c_str());
    }

    c.getBoolean(_USR_UNDECORATED_STATUSBAR_MSGS,undecoratedStatusbarMessages);
    bool dummyBool=false;
    if (c.getBoolean(_USR_CONSOLE_MSGS_TO_FILE,dummyBool))
        App::setConsoleMsgToFile(dummyBool);

    // Rendering section:
    // *****************************
    c.getInteger(_USR_IDLE_FPS,_idleFps);
    setIdleFps(_idleFps);
    c.getInteger(_USR_DESIRED_OPENGL_MAJOR,desiredOpenGlMajor);
    c.getInteger(_USR_DESIRED_OPENGL_MINOR,desiredOpenGlMinor);
    c.getInteger(_USR_OFFSCREEN_CONTEXT_TYPE,offscreenContextType);
    c.getInteger(_USR_FBO_TYPE,fboType);
    c.getBoolean(_USR_FORCE_FBO_VIA_EXT,forceFboViaExt);
    c.getInteger(_USR_VBO_OPERATION,vboOperation);
    c.getInteger(_USR_VBO_PERSISTENCE_IN_MS,vboPersistenceInMs);
    c.getBoolean(_USR_OGL_COMPATIBILITY_TWEAK_1,oglCompatibilityTweak1);
    c.getInteger(_USR_VISION_SENSORS_USE_GUI_WINDOWED,visionSensorsUseGuiThread_windowed);
    c.getInteger(_USR_VISION_SENSORS_USE_GUI_HEADLESS,visionSensorsUseGuiThread_headless);
    c.getBoolean(_USR_USE_GLFINISH,useGlFinish);
    c.getBoolean(_USR_USE_GLFINISH_VISION_SENSORS,useGlFinish_visionSensors);
    c.getInteger(_USR_VSYNC,vsync);
    c.getBoolean(_USR_DEBUG_OPENGL,debugOpenGl);
    c.getFloat(_USR_STEREO_DIST,stereoDist);
    c.getInteger(_USR_HIGH_RES_DISPLAY,highResDisplay);
    c.getBoolean(_USR_NO_EDGES_WHEN_MOUSE_DOWN,noEdgesWhenMouseDownInCameraView);
    c.getBoolean(_USR_NO_TEXTURES_WHEN_MOUSE_DOWN,noTexturesWhenMouseDownInCameraView);
    c.getBoolean(_USR_NO_CUSTOM_UIS_WHEN_MOUSE_DOWN,noCustomUisWhenMouseDownInCameraView);
    c.getInteger(_USR_HIERARCHY_REFRESH_CNT,hierarchyRefreshCnt);

    // Visual section:
    // *****************************
    c.getBoolean(_USR_DARK_MODE,darkMode);
    c.getInteger(_USR_RENDERING_SURFACE_VERTICAL_SHIFT,renderingSurfaceVShift);
    c.getInteger(_USR_RENDERING_SURFACE_VERTICAL_RESIZE,renderingSurfaceVResize);
    c.getBoolean(_USR_DISPLAY_WORLD_REF,displayWorldReference);
    c.getBoolean(_USR_ANTIALIASING,antiAliasing);
    c.getBoolean(_USR_DISPLAY_BOUNDING_BOXES,displayBoundingBoxeWhenObjectSelected);
    c.getInteger(_USR_GUI_FONT_SIZE_WIN,guiFontSize_Win);
    c.getInteger(_USR_GUI_FONT_SIZE_MAC,guiFontSize_Mac);
    c.getInteger(_USR_GUI_FONT_SIZE_LINUX,guiFontSize_Linux);
    c.getBoolean(_USR_STATUSBAR_INITIALLY_VISIBLE,statusbarInitiallyVisible);
    c.getBoolean(_USR_MODELBROWSER_INITIALLY_VISIBLE,modelBrowserInitiallyVisible);
#ifdef SIM_WITH_GUI
    //OLDMODELBROWSER COglSurface::_browserEnabled=modelBrowserInitiallyVisible;
    App::setBrowserEnabled(modelBrowserInitiallyVisible);
#endif
    c.getBoolean(_USR_SCENEHIERARCHY_INITIALLY_VISIBLE,sceneHierarchyInitiallyVisible);
#ifdef SIM_WITH_GUI
    COglSurface::_hierarchyEnabled=sceneHierarchyInitiallyVisible;
#endif
    c.getBoolean(_USR_SCENEHIERARCHY_HIDDEN_DURING_SIMULATION,sceneHierarchyHiddenDuringSimulation);

    c.getString(_USR_SCRIPT_EDITOR_FONT,scriptEditorFont);
    c.getBoolean(_USR_SCRIPT_EDITOR_BOLDFONT,scriptEditorBoldFont);
    c.getInteger(_USR_SCRIPT_EDITOR_FONT_SIZE,scriptEditorFontSize);

    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_BACKGROUND,mainScriptColor_background);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_SELECTION,mainScriptColor_selection);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_COMMENT,mainScriptColor_comment);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_NUMBER,mainScriptColor_number);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_STRING,mainScriptColor_string);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_CHARACTER,mainScriptColor_character);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_OPERATOR,mainScriptColor_operator);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_PREPROCESSOR,mainScriptColor_preprocessor);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_IDENTIFIER,mainScriptColor_identifier);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD,mainScriptColor_word);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD2,mainScriptColor_word2);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD3,mainScriptColor_word3);
    c.getIntVector3(_USR_MAIN_SCRIPT_COLOR_WORD4,mainScriptColor_word4);

    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND,nonThreadedChildScriptColor_background);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_SELECTION,nonThreadedChildScriptColor_selection);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_COMMENT,nonThreadedChildScriptColor_comment);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_NUMBER,nonThreadedChildScriptColor_number);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_STRING,nonThreadedChildScriptColor_string);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_CHARACTER,nonThreadedChildScriptColor_character);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_OPERATOR,nonThreadedChildScriptColor_operator);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR,nonThreadedChildScriptColor_preprocessor);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER,nonThreadedChildScriptColor_identifier);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD,nonThreadedChildScriptColor_word);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD2,nonThreadedChildScriptColor_word2);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD3,nonThreadedChildScriptColor_word3);
    c.getIntVector3(_USR_NON_THREADED_CHILD_SCRIPT_COLOR_WORD4,nonThreadedChildScriptColor_word4);

    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_BACKGROUND,threadedChildScriptColor_background);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_SELECTION,threadedChildScriptColor_selection);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_COMMENT,threadedChildScriptColor_comment);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_NUMBER,threadedChildScriptColor_number);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_STRING,threadedChildScriptColor_string);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_CHARACTER,threadedChildScriptColor_character);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_OPERATOR,threadedChildScriptColor_operator);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_PREPROCESSOR,threadedChildScriptColor_preprocessor);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_IDENTIFIER,threadedChildScriptColor_identifier);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD,threadedChildScriptColor_word);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD2,threadedChildScriptColor_word2);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD3,threadedChildScriptColor_word3);
    c.getIntVector3(_USR_THREADED_CHILD_SCRIPT_COLOR_WORD4,threadedChildScriptColor_word4);

    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_BACKGROUND,customizationScriptColor_background);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_SELECTION,customizationScriptColor_selection);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_COMMENT,customizationScriptColor_comment);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_NUMBER,customizationScriptColor_number);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_STRING,customizationScriptColor_string);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_CHARACTER,customizationScriptColor_character);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_OPERATOR,customizationScriptColor_operator);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_PREPROCESSOR,customizationScriptColor_preprocessor);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_IDENTIFIER,customizationScriptColor_identifier);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD,customizationScriptColor_word);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD2,customizationScriptColor_word2);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD3,customizationScriptColor_word3);
    c.getIntVector3(_USR_CUSTOMIZATION_SCRIPT_COLOR_WORD4,customizationScriptColor_word4);

    // Directories section:
    // *****************************
    c.getString(_USR_DIRECTORY_FOR_SCENES,defaultDirectoryForScenes);
    c.getString(_USR_DIRECTORY_FOR_MODELS,defaultDirectoryForModels);
    c.getString(_USR_DIRECTORY_FOR_CAD,defaultDirectoryForCadFiles);
    c.getString(_USR_DIRECTORY_FOR_MISC,defaultDirectoryForMiscFiles);

    // Serialization section:
    // *****************************
    c.getInteger(_USR_AUTO_SAVE_DELAY,autoSaveDelay);
    c.getBoolean(_USR_DO_NOT_WRITE_PERSISTENT_DATA,doNotWritePersistentData);
    c.getBoolean(_USR_COMPRESS_FILES,compressFiles);
    c.getBoolean(_USR_APPROXIMATED_NORMALS,saveApproxNormals);
    c.getBoolean(_USR_PACK_INDICES,packIndices);
    c.getInteger(_USR_FILE_DIALOGS_NATIVE,fileDialogs);
    c.getInteger(_USR_BULLET_SERIALIZATION_BUFFER,bulletSerializationBuffer);


    // Undo/redo section:
    // *****************************
    c.getBoolean(_USR_UNDO_REDO_ENABLED,_undoRedoEnabled);
    c.getBoolean(_USR_UNDO_REDO_PARTIAL_WITH_CAMERAS,_undoRedoOnlyPartialWithCameras);
    c.getInteger(_USR_UNDO_REDO_LEVEL_COUNT,undoRedoLevelCount);
    undoRedoLevelCount=tt::getLimitedInt(3,50,undoRedoLevelCount);
    c.getInteger(_USR_UNDO_REDO_MAX_BUFFER_SIZE,undoRedoMaxBufferSize);
    undoRedoMaxBufferSize=tt::getLimitedInt(2000000,800000000,undoRedoMaxBufferSize);
    c.getInteger(_USR_TIME_FOR_UNDO_REDO_TOO_LONG_WARNING,timeInMsForUndoRedoTooLongWarning);


    // Messaging section:
    // *****************************
    c.getBoolean(_USR_DO_NOT_SHOW_CRASH_RECOVERY_MESSAGE,doNotShowCrashRecoveryMessage);
    c.getBoolean(_USR_DO_NOT_SHOW_UPDATE_CHECK_MESSAGE,doNotShowUpdateCheckMessage);
    c.getBoolean(_USR_DO_NOT_SHOW_PROGRESS_BARS,doNotShowProgressBars);
    c.getBoolean(_USR_DO_NOT_SHOW_ACKNOWLEDGMENT_MESSAGES,doNotShowAcknowledgmentMessages);
    c.getBoolean(_USR_DO_NOT_SHOW_VIDEO_COMPRESSION_LIBRARY_LOAD_ERROR,doNotShowVideoCompressionLibraryLoadError);
    c.getBoolean(_USR_SUPPRESS_STARTUP_DIALOG,suppressStartupDialogs);
    c.getBoolean(_USR_SUPPRESS_XML_OVERWRITE_MSG,suppressXmlOverwriteMsg);


    // Compatibility section:
    // *****************************
    c.getBoolean(_USR_MIDDLE_MOUSE_BUTTON_SWITCHES_MODES,middleMouseButtonSwitchesModes);
    c.getBoolean(_USR_NAVIGATION_BACKWARD_COMPATIBILITY_MODE,navigationBackwardCompatibility);
    c.getFloat(_USR_COLOR_ADJUST_BACK_COMPATIBILITY,colorAdjust_backCompatibility);
    c.getBoolean(_USR_SPECIFIC_GPU_TWEAK,specificGpuTweak);
    c.getBoolean(_USR_USE_ALTERNATE_SERIAL_PORT_ROUTINES,useAlternateSerialPortRoutines);
    c.getBoolean(_USR_DISABLED_OPENGL_BASED_CUSTOM_UI,disableOpenGlBasedCustomUi);
    c.getBoolean(_USR_SHOW_old_DLGS,showOldDlgs);
    c.getBoolean(_USR_ENABLE_OLD_RENDERABLE,enableOldRenderableBehaviour);
    c.getBoolean(_USR_SUPPORT_old_THREADED_SCRIPTS,keepOldThreadedScripts);
    c.getBoolean(_USR_SUPPORT_old_API_NOTATION,_supportOldApiNotation);
    c.getBoolean(_USR_ENABLE_old_MIRROR_OBJECTS,enableOldMirrorObjects);
    c.getInteger(_USR_ALLOW_old_EDU_RELEASE,allowOldEduRelease);
    c.getInteger(_USR_THREADED_SCRIPTS_GRACE_TIME,threadedScriptsStoppingGraceTime);
    c.getInteger(_USR_BUGFIX1,bugFix1);
    c.getBoolean(_USR_COMPATIBILITYFIX1,compatibilityFix1);



    // Various section:
    // *****************************
    c.getFloat(_USR_MOUSE_WHEEL_ZOOM_FACTOR,mouseWheelZoomFactor);
    c.getFloat(_USR_DYNAMIC_ACTIVITY_RANGE,dynamicActivityRange);
    if (c.getFloat(_USR_TRANSLATION_STEP_SIZE,_translationStepSize))
        setTranslationStepSize(_translationStepSize);
    if (c.getFloat(_USR_ROTATION_STEP_SIZE,_rotationStepSize))
        setRotationStepSize(_rotationStepSize*degToRad_f);
    int processorCoreAffinity=0;
    if (c.getInteger(_USR_PROCESSOR_CORE_AFFINITY,processorCoreAffinity))
        CThreadPool_old::setProcessorCoreAffinity(processorCoreAffinity);
    c.getInteger(_USR_FREE_SERVER_PORT_START,freeServerPortStart);
    _nextfreeServerPortToUse=freeServerPortStart;
    c.getInteger(_USR_FREE_SERVER_PORT_RANGE,freeServerPortRange);
    c.getInteger(_USR_ABORT_SCRIPT_EXECUTION_BUTTON,_abortScriptExecutionButton);
    c.getInteger(_USR_TRIANGLE_COUNT_IN_OBB,triCountInOBB);
    c.getBoolean(_USR_REMOVE_IDENTICAL_VERTICES,identicalVerticesCheck);
    c.getFloat(_USR_IDENTICAL_VERTICES_TOLERANCE,identicalVerticesTolerance);
    c.getBoolean(_USR_REMOVE_IDENTICAL_TRIANGLES,identicalTrianglesCheck);
    c.getBoolean(_USR_TRIANGLE_WINDING_CHECK,identicalTrianglesWindingCheck);
    c.getBoolean(_USR_RUN_CUSTOMIZATION_SCRIPTS,runCustomizationScripts);
    c.getBoolean(_USR_TEST1,test1);
    c.getBoolean(_USR_ORDER_HIERARCHY_ALPHABETICALLY,orderHierarchyAlphabetically);
    c.getInteger(_USR_MAC_CHILD_DIALOG_TYPE,macChildDialogType);
#ifdef SIM_WITH_GUI
    #ifdef MAC_SIM
        if (macChildDialogType<=0)
        { // Qt::Tool
            VDialog::dialogStyle=QT_MODELESS_DLG_STYLE;
        }
        else
        { // Qt::Dialog
            VDialog::dialogStyle=Qt::Dialog;
        }
    #else
        VDialog::dialogStyle=QT_MODELESS_DLG_STYLE;
    #endif
#endif
    c.getString(_USR_ADDITIONAL_LUA_PATH,additionalLuaPath);
    c.getInteger(_USR_DESKTOP_RECORDING_INDEX,desktopRecordingIndex);
    c.getInteger(_USR_DESKTOP_RECORDING_WIDTH,desktopRecordingWidth);
    c.getBoolean(_USR_FORCE_BUG_FIX_REL_30002,forceBugFix_rel30002);
    c.getString(_USR_EXTERNAL_SCRIPT_EDITOR,externalScriptEditor);
    c.getInteger(_USR_XML_EXPORT_SPLIT_SIZE,xmlExportSplitSize);
    c.getBoolean(_USR_XML_EXPORT_KNOWN_FORMATS,xmlExportKnownFormats);

    // Floating license / dongle license section:
    // *****************************
    c.getBoolean(_USR_FLOAT_LICENSE_ENABLED,floatingLicenseEnabled);
    c.getString(_USR_FLOAT_LICENSE_SERVER_ADDRESS,floatingLicenseServer);
    c.getInteger(_USR_FLOAT_LICENSE_SERVER_PORT,floatingLicensePort);
    c.getBoolean(_USR_KEEP_DONGLE_OPEN,keepDongleOpen);
    c.getInteger(_USR_XR_TEST,xrTest);
}

