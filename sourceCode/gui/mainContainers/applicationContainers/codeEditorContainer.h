#pragma once

#include <vector>
#include <QString>
#include "tinyxml2.h"

struct SCodeEditor {
    int handle;
    int uniqueId;
    int scriptHandle;
    int callingScriptHandle;
    int sceneUniqueId;
    bool openAcrossScenes;
    bool closeAtSimulationEnd;
    bool systemVisibility;
    bool userVisibility;
    bool closeAfterCallbackCalled;
    bool restartScriptWhenClosing;
    std::string callbackFunction;
};


class CCodeEditorContainer
{
public:
    CCodeEditorContainer();
    virtual ~CCodeEditorContainer();

    // From UI thread:
    void showOrHideAll(bool showState);
    void closeAll();

    // From sim thread:
    int openScriptWithExternalEditor(int scriptHandle);

    int open(const char* initText,const char* xml,int callingScriptHandle); // func. sim.textEditorOpen
    int openSimulationScript(int scriptHandle,int callingScriptHandle); // main and child scripts
    int openCustomizationScript(int scriptHandle,int callingScriptHandle); // customization scripts
    int openConsole(const char* title,int maxLines,int mode,const int position[2],const int size[2],const int textColor[3],const int backColor[3],int callingScriptHandle);
    std::string openModalTextEditor(const char* initText,const char* xml,int windowSizeAndPos[4]) const; // modal C func. sim.openTextEditor
    int openTextEditor(const char* initText,const char* xml,const char* callback,int callingScriptHandle,bool isSimulationScript); // deprec. func. sim.openTextEditor
    bool close(int handle,int posAndSize[4],std::string* txt,std::string* callback);
    void applyChanges(int handle) const;
    bool closeFromScriptHandle(int scriptHandle,int posAndSize[4],bool ignoreChange);
    void restartScript(int handle) const;
    void resetScript(int scriptHandle) const;
    int getCallingScriptHandle(int handle) const;
    bool getCloseAfterCallbackCalled(int handle) const;
    void simulationAboutToStart() const;
    void simulationAboutToEnd();
    void saveOrCopyOperationAboutToHappen() const;
    bool setText(int handle,const char* txt) const;
    std::string getText(int handle,int posAndSize[4]) const;
    bool isHandleValid(int handle) const;
    int showOrHide(int handle,bool showState);
    int getShowState(int handle) const;
    bool appendText(int handle,const char* txt) const;
    bool hasSomethingBeenModifiedInCurrentScene() const;

    // From any thread:
    int getHandleFromUniqueId(int uid);
    int getUniqueId(int handle);
    bool areSceneEditorsOpen() const;
    void sceneClosed(int sceneUniqueId);
    static std::string getColorStr(const int rgbCol[3]);
    static void getKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded);
    static void getFuncKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded);
    static void getVarKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded);
    static std::string translateXml(const char* oldXml,const char* callback);
    static const char* toBoolStr(bool v);

protected:
    std::vector<SCodeEditor> _allEditors;
    static int _nextUniqueId;
};
