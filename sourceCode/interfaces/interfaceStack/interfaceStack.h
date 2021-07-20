#pragma once

#include "interfaceStackObject.h"
#include "interfaceStackTable.h"
#include <vector>
#include <string>
#include <map>

class CInterfaceStack
{
public:
    CInterfaceStack();
    virtual ~CInterfaceStack();

    void setId(int id);
    int getId();
    void clear();

    // C interface (creation):
    CInterfaceStack* copyYourself() const;
    void copyFrom(const CInterfaceStack* source);
    void pushObjectOntoStack(CInterfaceStackObject* obj);
    void pushNullOntoStack();
    void pushBoolOntoStack(bool v);
    void pushNumberOntoStack(double v);
    void pushInt32OntoStack(int v);
    void pushInt64OntoStack(long long int v);
    void pushStringOntoStack(const char* str,size_t l);
    void pushUCharArrayTableOntoStack(const unsigned char* arr,int l);
    void pushInt32ArrayTableOntoStack(const int* arr,int l);
    void pushInt64ArrayTableOntoStack(const long long int* arr,int l);
    void pushFloatArrayTableOntoStack(const float* arr,int l);
    void pushDoubleArrayTableOntoStack(const double* arr,int l);
    void pushTableOntoStack();
    bool insertDataIntoStackTable();
    bool pushTableFromBuffer(const char* data,unsigned int l);


    // C interface (read-out)
    int getStackSize() const;
    void popStackValue(int cnt);
    bool moveStackItemToTop(int cIndex);
    CInterfaceStackObject* getStackObjectFromIndex(size_t index) const;
    bool getStackBoolValue(bool& theValue) const;
    bool getStackStrictNumberValue(double& theValue) const;
    bool getStackStrictIntegerValue(long long int& theValue) const;
    bool getStackDoubleValue(double& theValue) const;
    bool getStackFloatValue(float& theValue) const;
    bool getStackInt64Value(long long int& theValue) const;
    bool getStackInt32Value(int& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array,int count) const;
    bool getStackInt32Array(int* array,int count) const;
    bool getStackInt64Array(long long int* array,int count) const;
    bool getStackFloatArray(float* array,int count) const;
    bool getStackDoubleArray(double* array,int count) const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const char* fieldName) const;
    bool getStackMapBoolValue(const char* fieldName,bool& val) const;
    bool getStackMapStrictNumberValue(const char* fieldName,double& val) const;
    bool getStackMapStrictIntegerValue(const char* fieldName,long long int& val) const;
    bool getStackMapDoubleValue(const char* fieldName,double& val) const;
    bool getStackMapFloatValue(const char* fieldName,float& val) const;
    bool getStackMapLongIntValue(const char* fieldName,long long int& val) const;
    bool getStackMapIntValue(const char* fieldName,int& val) const;
    bool getStackMapStringValue(const char* fieldName,std::string& val) const;
    bool getStackMapFloatArray(const char* fieldName,float* array,int count) const;
    std::string getBufferFromTable() const;

    void printContent(int cIndex,std::string& buffer) const;

protected:
    int _interfaceStackId;
    std::vector<CInterfaceStackObject*> _stackObjects;
};
