#include "confReaderAndWriter.h"
#include "tt.h"
#include "vFile.h"
#include "vArchive.h"
#include <boost/lexical_cast.hpp>

CConfReaderAndWriter::CConfReaderAndWriter()
{
}

CConfReaderAndWriter::~CConfReaderAndWriter()
{
}

bool CConfReaderAndWriter::readConfiguration(const char* filename)
{
    bool retVal=false;
    if (VFile::doesFileExist(filename))
    {
        try
        {
            VFile file(filename,VFile::READ|VFile::SHARE_DENY_NONE);
            VArchive archive(&file,VArchive::LOAD);
            unsigned int actualPosition=0;
            std::string line;
            while (archive.readSingleLine(actualPosition,line,false))
            {
                tt::removeComments(line);
                size_t ePos=line.find('=');
                if ( (ePos!=std::string::npos)&&(ePos>0)&&(ePos+1<line.length()) )
                {
                    std::string var,val;
                    var.assign(line.begin(),line.begin()+ePos);
                    val.assign(line.begin()+ePos+1,line.end());
                    if ( tt::removeSpacesAtBeginningAndEnd(var)&&tt::removeSpacesAtBeginningAndEnd(val)&&(var.find(' ')==std::string::npos) )
                    {
                        int ind=_getVariableIndex((const char*)var.c_str());
                        if (ind!=-1)
                        { // Variable multiple defined. We keep the last one!
                            _variables.erase(_variables.begin()+ind);
                            _values.erase(_values.begin()+ind);
                            _comments.erase(_comments.begin()+ind);
                        }
                        _variables.push_back(var);
                        _values.push_back(val);
                        _comments.push_back("");
                    }
                }
            }
            archive.close();
            file.close();
            retVal=true;
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
            retVal=false;
        }
    }
    return(retVal);
}

bool CConfReaderAndWriter::writeConfiguration(const char* filename)
{
    bool retVal=false;
    try
    {
        VFile myFile(filename,VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
        VArchive archive(&myFile,VArchive::STORE);

        std::string line;
        for (int i=0;i<int(_variables.size());i++)
        {
            if (_variables[i].length()>0)
            {
                line=_variables[i]+" = ";
                line+=_values[i];
                if (_comments[i]!="")
                {
                    line+=" // ";
                    line+=_comments[i];
                }
            }
            else
                line=_comments[i];
            archive.writeLine(line);
        }
        archive.close();
        myFile.close();
        retVal=true;
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        VFile::reportAndHandleFileExceptionError(e);
        retVal=false;
    }
    return(retVal);
}

bool CConfReaderAndWriter::getString(const char* variableName,std::string& variable)
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    variable=_values[ind];
    return(true);
}

bool CConfReaderAndWriter::getBoolean(const char* variableName,bool& variable)
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    std::string val=_values[ind];
    for (int i=0;i<int(val.length());i++)
    {
        if (val[i]>='a')
            val[i]-=('a'-'A');
    }
    if (val.compare("TRUE")==0)
    {
        variable=true;
        return(true);
    }
    if (val.compare("FALSE")==0)
    {
        variable=false;
        return(true);
    }
    return(false);
}


bool CConfReaderAndWriter::getInteger(const char* variableName,int& variable)
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    return(tt::getValidInt(_values[ind].c_str(),variable));
}

bool CConfReaderAndWriter::getFloat(const char* variableName,float& variable)
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    return(tt::getValidFloat(_values[ind].c_str(),variable));
}

bool CConfReaderAndWriter::getFloatVector3(const char* variableName,float variable[3])
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    std::string v=_values[ind];
    size_t cPos=v.find(',');
    std::string tmp;
    if ( (cPos!=std::string::npos)&&(cPos>0)&&(cPos+1<v.length()) )
    {
        tmp.assign(v.begin(),v.begin()+cPos);
        if (!tt::getValidFloat(tmp.c_str(),variable[0]))
            return(false);
        tmp.assign(v.begin()+cPos+1,v.end());
        v=tmp;
        cPos=v.find(',');
        if ( (cPos!=std::string::npos)&&(cPos>0)&&(cPos+1<v.length()) )
        {
            tmp.assign(v.begin(),v.begin()+cPos);
            if (!tt::getValidFloat(tmp.c_str(),variable[1]))
                return(false);
            tmp.assign(v.begin()+cPos+1,v.end());
            return(tt::getValidFloat(tmp.c_str(),variable[2]));
        }
    }
    return(false);
}

bool CConfReaderAndWriter::getIntVector3(const char* variableName,int variable[3])
{
    int ind=_getVariableIndex(variableName);
    if (ind==-1)
        return(false);
    std::string v=_values[ind];
    size_t cPos=v.find(',');
    std::string tmp;
    if ( (cPos!=std::string::npos)&&(cPos>0)&&(cPos+1<v.length()) )
    {
        tmp.assign(v.begin(),v.begin()+cPos);
        if (!tt::getValidInt(tmp.c_str(),variable[0]))
            return(false);
        tmp.assign(v.begin()+cPos+1,v.end());
        v=tmp;
        cPos=v.find(',');
        if ( (cPos!=std::string::npos)&&(cPos>0)&&(cPos+1<v.length()) )
        {
            tmp.assign(v.begin(),v.begin()+cPos);
            if (!tt::getValidInt(tmp.c_str(),variable[1]))
                return(false);
            tmp.assign(v.begin()+cPos+1,v.end());
            return(tt::getValidInt(tmp.c_str(),variable[2]));
        }
    }
    return(false);
}

bool CConfReaderAndWriter::addString(const char* variableName,std::string variable,const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    _values.push_back(variable);
    return(true);
}

bool CConfReaderAndWriter::addInteger(const char* variableName,int variable,const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    _values.push_back(boost::lexical_cast<std::string>(variable));
    return(true);
}

bool CConfReaderAndWriter::addFloat(const char* variableName,float variable,const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    _values.push_back(boost::lexical_cast<std::string>(variable));
    return(true);
}

bool CConfReaderAndWriter::addRandomLine(const char* lineText)
{
    _variables.push_back("");
    _comments.push_back(lineText);
    _values.push_back("");
    return(true);
}

bool CConfReaderAndWriter::addFloatVector3(const char* variableName,float variable[3],const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    std::string tmp=boost::lexical_cast<std::string>(variable[0])+",";
    tmp+=boost::lexical_cast<std::string>(variable[1])+",";
    tmp+=boost::lexical_cast<std::string>(variable[2]);
    _values.push_back(tmp);
    return(true);
}

bool CConfReaderAndWriter::addIntVector3(const char* variableName,int variable[3],const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    std::string tmp=boost::lexical_cast<std::string>(variable[0])+",";
    tmp+=boost::lexical_cast<std::string>(variable[1])+",";
    tmp+=boost::lexical_cast<std::string>(variable[2]);
    _values.push_back(tmp);
    return(true);
}

bool CConfReaderAndWriter::addBoolean(const char* variableName,bool variable,const char* comment)
{
    if (_getVariableIndex(variableName)!=-1)
        return(false); // variable already present
    _variables.push_back(variableName);
    _comments.push_back(comment);
    if (variable)
        _values.push_back("true");
    else
        _values.push_back("false");
    return(true);
}

int CConfReaderAndWriter::_getVariableIndex(const char* variableName)
{
    for (int i=0;i<int(_variables.size());i++)
    {
        if (_variables[i].compare(variableName)==0)
            return(i);
    }
    return(-1);
}
