#include "pathPlanning_old.h"
#include "pathPlanningInterface.h"
#include "simInternal.h"

CPathPlanning_old::CPathPlanning_old()
{
}

CPathPlanning_old::~CPathPlanning_old()
{
}

int CPathPlanning_old::searchPath(int maxTimePerPass)
{ // maxTimePerPass is in miliseconds
    return(false);
}
bool CPathPlanning_old::setPartialPath()
{
    return(false);
}
int CPathPlanning_old::smoothFoundPath(int steps,int maxTimePerPass)
{ // step specifies the smoothing factor
    return(0);
}

void CPathPlanning_old::getPathData(std::vector<float>& data)
{
}

void CPathPlanning_old::getSearchTreeData(std::vector<float>& data,bool fromStart)
{
}

bool CPathPlanning_old::doCollide(float* dist)
{ // dist can be nullptr
    return(false);
}

