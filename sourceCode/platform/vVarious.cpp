
#include "vVarious.h"
#include "tt.h"
#include "vFile.h"
#include "vArchive.h"
#include "ttUtil.h"
#include "vDateTime.h"
#include <string.h>
#ifdef WIN_SIM
    #include <Windows.h>
    #include <Shellapi.h>
#else
    #include <unistd.h>
#endif
#ifdef SIM_WITH_GUI
    #include <QApplication>
    #include <QClipboard>
    #include <QDesktopServices>
#endif
#ifdef SIM_WITH_QT
    #include <QUrl>
    #include <QProcess>
#else
    #ifndef WIN_SIM
        #include <dlfcn.h>
        #include <stdlib.h>
    #endif
#endif

bool VVarious::executeExternalApplication(const char* file,const char* arguments,const char* switchToDirectory,int showFlag)
{
#ifdef WIN_SIM
    int sh=SW_SHOWDEFAULT;
    if (showFlag==VVARIOUS_SHOWMAXIMIZED)
        sh=SW_SHOWMAXIMIZED;
    if (showFlag==VVARIOUS_HIDE)
        sh=SW_HIDE;
    if (strlen(switchToDirectory)!=0)
        SetCurrentDirectoryA(switchToDirectory); // needed because otherwise the shellExecute command might switch directories!

    std::string cmd(file);
    if (strlen(file)>0)
    {
        if (file[0]=='@')
            cmd.erase(cmd.begin());
    }
    return (reinterpret_cast<long long>(ShellExecuteA(nullptr,"open",cmd.c_str(),arguments,nullptr,sh))>32);
#else // WIN_SIM
#ifndef SIM_WITH_QT
    std::string cmd(file);
    if (strlen(file)>0)
    {
        if (file[0]!='@')
        {
            if (std::string(file).find('/')==std::string::npos)
            {
                cmd=switchToDirectory;
                cmd+="/";
            }
            cmd+=file;
        }
        else
        { // for system-wide commands (e.g. xdg-open)
            cmd=file;
            cmd.erase(cmd.begin());
        }
    }

    pid_t pid;
    std::vector<std::string> args;
    std::string w;
    std::string argu(arguments);
    while (tt::extractSpaceSeparatedWord2(argu,w,true,true,false,false,false))
        args.push_back(w);
    while (args.size()<10)
    args.push_back("");
    pid=fork();
    if (pid==0)
    {
        execl(cmd.c_str(),cmd.c_str(),args[0].c_str(),args[1].c_str(),args[2].c_str(),args[3].c_str(),args[4].c_str(),args[5].c_str(),args[6].c_str(),args[7].c_str(),args[8].c_str(),args[9].c_str(),(char*)0);
        exit(0);
    }
    return(true);
#else
    QString cmd;
    if (strlen(file)>0)
    {
        if (file[0]!='@')
        {
            if (std::string(file).find('/')==std::string::npos)
                cmd="./";
            cmd+=QString::fromLocal8Bit(file);
        }
        else
        { // for system-wide commands (e.g. xdg-open)
            cmd=QString::fromLocal8Bit(file);
            cmd.remove(0,1);
        }
    }
    QStringList strList;
    std::string args(arguments);
    std::string word;
    while (tt::extractSpaceSeparatedWord(args,word))
        strList << QString(word.c_str());
    return(QProcess::startDetached(cmd,strList,QString::fromLocal8Bit(switchToDirectory),nullptr));
#endif
#endif // WIN_SIM
    /*
    boost::process::opstream sin;
    boost::process::ipstream sout;
    boost::filesystem::path program;
    if (true)//in->useSearchPath)
        program = boost::process::search_path(file);
    else
        program = file;
    boost::process::child child(program, arguments, boost::process::std_in = sin, boost::process::std_out > sout);
    sin << "";
    sin.flush();
    sin.pipe().close();
    child.wait();
    printf("ExitCode: %i\n",child.exit_code());
    std::string s(std::istreambuf_iterator<char>(sout), {});
    printf("Ret: %s\n",s.c_str());
    */
}

std::string VVarious::getModulePath()
{
    static std::string retVal;
    if (retVal.size()==0)
    {
        char curDirAndFile[2048];
#ifdef _WIN32
        GetModuleFileNameA(nullptr,curDirAndFile,2000);
        int i=0;
        while (true)
        {
            if (curDirAndFile[i]==0)
                break;
            if (curDirAndFile[i]=='\\')
                curDirAndFile[i]='/';
            i++;
        }
        retVal=splitPath_path(curDirAndFile);
#else
        getcwd(curDirAndFile, 2000);
        retVal=curDirAndFile;
#endif
    }
    return(retVal);
}

void VVarious::removePathFinalSlashOrBackslash(std::string& pathWithOrWithoutFinalThing)
{
    size_t l=pathWithOrWithoutFinalThing.length();
    if (l>0)
    {
        if ((pathWithOrWithoutFinalThing[l-1]=='/')||(pathWithOrWithoutFinalThing[l-1]=='\\'))
        { // this might be a / or \\ part of a double char. Check for that:
            bool removeIt=true;
            if (l>1)
            {
                if ((unsigned char)(pathWithOrWithoutFinalThing[l-2])>0x7F)
                    removeIt=false;
            }

            if (removeIt)
                pathWithOrWithoutFinalThing.erase(pathWithOrWithoutFinalThing.end()-1);
        }
    }
}

std::string VVarious::splitPath_path(const char* fullPathAndName)
{   // returns the absolute path, without a final / or backslash
    std::string retVal;
#ifndef SIM_WITH_QT
    // TODO_SIM_WITH_QT
    // This routine should be rewritten with OS-specific mechanisms
    retVal=fullPathAndName;
    while ( (retVal.size()>0)&&(retVal[retVal.size()-1]!='/')&&(retVal[retVal.size()-1]!='\\') )
        retVal.erase(retVal.end()-1);
    if (retVal.size()>0)
        retVal.erase(retVal.end()-1);
#else
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.path().toLocal8Bit().data();
    size_t l=retVal.length();
    if (l!=0)
    {
        if ((retVal[l-1]=='/')||(retVal[l-1]=='\\'))
        { // this might be a / or \\ part of a double char. Check for that:
            bool removeIt=true;
            if (l>1)
            {
                if ((unsigned char)(retVal[l-2])>0x7F)
                    removeIt=false;
            }

            if (removeIt)
                retVal.erase(retVal.end()-1);
        }
    }
#endif
    return(retVal);
}

std::string VVarious::splitPath_fileBaseAndExtension(const char* fullPathAndName)
{ // returns the filename including extension
    std::string retVal;
#ifndef SIM_WITH_QT
    // TODO_SIM_WITH_QT
    // This routine should be rewritten with OS-specific mechanisms
    std::string tmp(fullPathAndName);
    while ( (tmp.size()>0)&&(tmp[tmp.size()-1]!='/')&&(tmp[tmp.size()-1]!='\\')&&(tmp[tmp.size()-1]!=':') )
    {
        retVal=tmp[tmp.size()-1]+retVal;
        tmp.erase(tmp.end()-1);
    }
#else
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.fileName().toLocal8Bit().data();
#endif
    return(retVal);
}


std::string VVarious::splitPath_fileBase(const char* fullPathAndName)
{   // returns the base of a filename, without path or extension. for xxx/yyy/zzz.a.b.c will return zzz.a.b
    std::string retVal;
#ifndef SIM_WITH_QT
    // TODO_SIM_WITH_QT
    // This routine should be rewritten with OS-specific mechanisms
    retVal=splitPath_fileBaseAndExtension(fullPathAndName);
    std::string tmp(retVal);
    while (tmp.size()>0)
    {
        if (tmp[tmp.size()-1]=='.')
        {
            tmp.erase(tmp.end()-1);
            retVal=tmp;
            break;
        }
        tmp.erase(tmp.end()-1);
    }
#else
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.completeBaseName().toLocal8Bit().data();
#endif
    return(retVal);
}

std::string VVarious::splitPath_fileExtension(const char* fullPathAndName)
{   // returns the filename extension (without '.')
    std::string retVal;
#ifndef SIM_WITH_QT
    // TODO_SIM_WITH_QT
    // This routine should be rewritten with OS-specific mechanisms
    std::string tmp(fullPathAndName);
    bool hadDot=false;
    while ( (tmp.size()>0)&&(tmp[tmp.size()-1]!='/')&&(tmp[tmp.size()-1]!='\\')&&(tmp[tmp.size()-1]!=':') )
    {
        if (tmp[tmp.size()-1]=='.')
        {
            hadDot=true;
            break;
        }
        retVal=tmp[tmp.size()-1]+retVal;
        tmp.erase(tmp.end()-1);
    }
    if (!hadDot)
        retVal.clear();
#else
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.suffix().toLocal8Bit().data();
#endif
    return(retVal);
}

bool VVarious::isAbsolutePath(const char* pathAndOptionalFilename)
{
#ifndef SIM_WITH_QT
    // TODO_SIM_WITH_QT
    // This routine should be rewritten with OS-specific mechanisms
#ifdef WIN_SIM
    if (strlen(pathAndOptionalFilename)==0)
        return(false);
    if (pathAndOptionalFilename[0]=='\\')
        return(true);
    if (strlen(pathAndOptionalFilename)<2)
        return(false);
    if (pathAndOptionalFilename[1]==':')
        return(true);
    return(false);
#else // WIN_SIM
    if (strlen(pathAndOptionalFilename)==0)
        return(false);
    return(pathAndOptionalFilename[0]=='/');
#endif // WIN_SIM
#else
    QFileInfo pathInfo(QString::fromLocal8Bit(pathAndOptionalFilename));
    return(pathInfo.isAbsolute());
#endif
}

WLibrary VVarious::openLibrary(const char* filename)
{ // here we have the extension in the filename (.dll, .so or .dylib)
#ifndef SIM_WITH_QT
#ifdef WIN_SIM
    return LoadLibraryA(filename);
#else
    return dlopen(filename,RTLD_LAZY);
#endif
#else
    WLibrary lib=new QLibrary(filename);
    if (!lib->load())
    {
        delete lib;
        lib=nullptr;
    }
    return(lib);
#endif
}

void VVarious::closeLibrary(WLibrary lib)
{
#ifndef SIM_WITH_QT
#ifdef WIN_SIM
    if (lib!=0)
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif
#else
    if (lib!=nullptr)
    {
        lib->unload();
        delete lib;
    }
#endif
}

WLibraryFunc VVarious::resolveLibraryFuncName(WLibrary lib,const char* funcName)
{
#ifndef SIM_WITH_QT
#ifdef WIN_SIM
    if (lib!=nullptr)
        return GetProcAddress(lib,funcName);
#else
    if (lib!=nullptr)
        return dlsym(lib,funcName);
#endif
#else
    if (lib!=nullptr)
        return((void*)lib->resolve(funcName));
#endif
    return(nullptr);
}

bool VVarious::copyTextToClipboard(const char* text)
{
#ifdef SIM_WITH_GUI
    QClipboard *clipboard=QApplication::clipboard();
    clipboard->setText(text);
#endif
    return(true);
}

#ifdef SIM_WITH_GUI
bool VVarious::openOfflineUrl(const char* url)
{
    return(QDesktopServices::openUrl(QUrl::fromLocalFile(url)));
}

bool VVarious::openOnlineUrl(const char* url)
{
    return(QDesktopServices::openUrl(QUrl(url)));
}

bool VVarious::openTextFile(const char* file)
{
    return(QDesktopServices::openUrl(QUrl::fromLocalFile(file)));
}
#endif
