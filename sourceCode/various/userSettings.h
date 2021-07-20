#pragma once

#include "colorObject.h"

class CUserSettings
{
public:
    CUserSettings();
    virtual ~CUserSettings();

    static bool getBooleanFromFileDirectly(const char* varName,bool& varValue);
    static bool getIntegerFromFileDirectly(const char* varName,int& varValue);
    static bool getFloatFromFileDirectly(const char* varName,float& varValue);
    static bool getStringFromFileDirectly(const char* varName,std::string& varValue);

    void setUndoRedoEnabled(bool isEnabled);
    bool getUndoRedoEnabled();
    bool getUndoRedoOnlyPartialWithCameras();

    void saveUserSettings();
    void loadUserSettings();

    void setTranslationStepSize(float s);
    float getTranslationStepSize();
    void setRotationStepSize(float s);
    float getRotationStepSize();
    int getIdleFps();
    void setIdleFps(int fps);
    int getAbortScriptExecutionTiming();


    int getNextFreeServerPortToUse();
    void setNextFreeServerPortToUse(int p);

    bool getSupportOldApiNotation();

    std::string _overrideConsoleVerbosity;
    std::string _overrideStatusbarVerbosity;
    std::string _consoleLogFilter;
    std::string _overrideDialogVerbosity;
    bool undecoratedStatusbarMessages;
    bool displayWorldReference;
    bool useGlFinish;
    bool useGlFinish_visionSensors;
    bool oglCompatibilityTweak1;
    float stereoDist;
    int vsync;
    bool debugOpenGl;
    bool identicalVerticesCheck;
    float identicalVerticesTolerance;
    bool identicalTrianglesCheck;
    bool identicalTrianglesWindingCheck;
    bool compressFiles;
    int triCountInOBB;
    bool saveApproxNormals;
    bool packIndices;
    bool runCustomizationScripts;
    bool test1;
    bool orderHierarchyAlphabetically;
    int macChildDialogType;

    int undoRedoLevelCount;
    int undoRedoMaxBufferSize;
    bool alwaysShowConsole;
    bool forceBugFix_rel30002;
    bool statusbarInitiallyVisible;
    bool modelBrowserInitiallyVisible;
    bool sceneHierarchyInitiallyVisible;
    bool sceneHierarchyHiddenDuringSimulation;
    int autoSaveDelay;
    int bulletSerializationBuffer;
    int timeInMsForUndoRedoTooLongWarning;
    int bugFix1;
    bool compatibilityFix1;

    bool middleMouseButtonSwitchesModes;
    bool navigationBackwardCompatibility;
    float colorAdjust_backCompatibility;
    bool specificGpuTweak;
    bool useAlternateSerialPortRoutines;
    bool disableOpenGlBasedCustomUi;
    bool showOldDlgs;
    bool enableOldRenderableBehaviour;
    bool keepOldThreadedScripts;
    bool enableOldMirrorObjects;
    int allowOldEduRelease;
    int threadedScriptsStoppingGraceTime;
    bool displayBoundingBoxeWhenObjectSelected;
    bool antiAliasing;
    float dynamicActivityRange;
    int freeServerPortStart;
    int freeServerPortRange;
    bool darkMode;
    int renderingSurfaceVShift;
    int renderingSurfaceVResize;
    int offscreenContextType;
    int fboType;
    bool forceFboViaExt;
    int vboOperation;
    int vboPersistenceInMs;
    int desiredOpenGlMajor;
    int desiredOpenGlMinor;
    int visionSensorsUseGuiThread_windowed;
    int visionSensorsUseGuiThread_headless;
    int fileDialogs;
    float mouseWheelZoomFactor;

    bool doNotWritePersistentData;
    bool doNotShowCrashRecoveryMessage;
    bool doNotShowUpdateCheckMessage;
    bool doNotShowProgressBars;
    bool doNotShowAcknowledgmentMessages;
    bool doNotShowVideoCompressionLibraryLoadError;
    bool suppressStartupDialogs;
    bool suppressXmlOverwriteMsg;

    std::string scriptEditorFont;
    bool scriptEditorBoldFont;
    int scriptEditorFontSize;

    std::string additionalLuaPath;

    int guiFontSize_Win;
    int guiFontSize_Mac;
    int guiFontSize_Linux;

    std::string defaultDirectoryForScenes;
    std::string defaultDirectoryForModels;
    std::string defaultDirectoryForCadFiles;
    std::string defaultDirectoryForMiscFiles;

    int threadedChildScriptColor_background[3];
    int threadedChildScriptColor_selection[3];
    int threadedChildScriptColor_comment[3];
    int threadedChildScriptColor_number[3];
    int threadedChildScriptColor_string[3];
    int threadedChildScriptColor_character[3];
    int threadedChildScriptColor_operator[3];
    int threadedChildScriptColor_preprocessor[3];
    int threadedChildScriptColor_identifier[3];
    int threadedChildScriptColor_word[3];
    int threadedChildScriptColor_word2[3];
    int threadedChildScriptColor_word3[3];
    int threadedChildScriptColor_word4[3];

    int nonThreadedChildScriptColor_background[3];
    int nonThreadedChildScriptColor_selection[3];
    int nonThreadedChildScriptColor_comment[3];
    int nonThreadedChildScriptColor_number[3];
    int nonThreadedChildScriptColor_string[3];
    int nonThreadedChildScriptColor_character[3];
    int nonThreadedChildScriptColor_operator[3];
    int nonThreadedChildScriptColor_preprocessor[3];
    int nonThreadedChildScriptColor_identifier[3];
    int nonThreadedChildScriptColor_word[3];
    int nonThreadedChildScriptColor_word2[3];
    int nonThreadedChildScriptColor_word3[3];
    int nonThreadedChildScriptColor_word4[3];

    int mainScriptColor_background[3];
    int mainScriptColor_selection[3];
    int mainScriptColor_comment[3];
    int mainScriptColor_number[3];
    int mainScriptColor_string[3];
    int mainScriptColor_character[3];
    int mainScriptColor_operator[3];
    int mainScriptColor_preprocessor[3];
    int mainScriptColor_identifier[3];
    int mainScriptColor_word[3];
    int mainScriptColor_word2[3];
    int mainScriptColor_word3[3];
    int mainScriptColor_word4[3];

    int customizationScriptColor_background[3];
    int customizationScriptColor_selection[3];
    int customizationScriptColor_comment[3];
    int customizationScriptColor_number[3];
    int customizationScriptColor_string[3];
    int customizationScriptColor_character[3];
    int customizationScriptColor_operator[3];
    int customizationScriptColor_preprocessor[3];
    int customizationScriptColor_identifier[3];
    int customizationScriptColor_word[3];
    int customizationScriptColor_word2[3];
    int customizationScriptColor_word3[3];
    int customizationScriptColor_word4[3];

    int highResDisplay;
    bool noEdgesWhenMouseDownInCameraView;
    bool noTexturesWhenMouseDownInCameraView;
    bool noCustomUisWhenMouseDownInCameraView;
    int hierarchyRefreshCnt;
    int desktopRecordingIndex;
    int desktopRecordingWidth;
    std::string externalScriptEditor;
    int xmlExportSplitSize;
    bool xmlExportKnownFormats;

    bool floatingLicenseEnabled;
    std::string floatingLicenseServer;
    int floatingLicensePort;
    bool keepDongleOpen;
    int xrTest;

private:
    bool _supportOldApiNotation;
    void _setIntVector3(int v[3],int a,int b,int c);
    bool _undoRedoEnabled;
    bool _undoRedoOnlyPartialWithCameras;
    float _translationStepSize;
    float _rotationStepSize;
    int _idleFps;
    int _nextfreeServerPortToUse;
    int _abortScriptExecutionButton;
};
