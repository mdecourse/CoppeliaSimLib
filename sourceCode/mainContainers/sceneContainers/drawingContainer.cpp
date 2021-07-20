
#include "simInternal.h"
#include "drawingContainer.h"
#include "viewableBase.h"
#include "easyLock.h"

CDrawingContainer::CDrawingContainer()
{
}

CDrawingContainer::~CDrawingContainer()
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        delete _allObjects[i];
        _allObjects.erase(_allObjects.begin()+i);
    }
}

void CDrawingContainer::simulationEnded()
{
}

CDrawingObject* CDrawingContainer::getObject(int objectID)
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
            return(_allObjects[i]);
    }
    return(nullptr);
}


int CDrawingContainer::addObject(CDrawingObject* it)
{
    EASYLOCK(_objectMutex);
    int newID=0;
    newID++;
    while (getObject(newID)!=nullptr)
        newID++;
    it->setObjectID(newID);
    _allObjects.push_back(it);
    return(newID);
}

void CDrawingContainer::removeObject(int objectID)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]->getObjectID()==objectID)
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
            break;
        }
    }
}

bool CDrawingContainer::getExportableMeshAtIndex(int parentObjectID,int index,std::vector<float>& vertices,std::vector<int>& indices)
{
    vertices.clear();
    indices.clear();
    int cnt=0;
    for (int i=0;i<int(_allObjects.size());i++)
    {
        if ((_allObjects[i]->getSceneObjectID()==parentObjectID)&&_allObjects[i]->canMeshBeExported())
        {
            cnt++;
            if (cnt==index+1)
            {
                _allObjects[i]->getExportableMesh(vertices,indices);
                return(true);
            }
        }
    }
    return(false);
}

void CDrawingContainer::adjustForFrameChange(int objectID,const C7Vector& preCorrection)
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]->getSceneObjectID()==objectID)
            _allObjects[i]->adjustForFrameChange(preCorrection);
    }
}

void CDrawingContainer::adjustForScaling(int objectID,float xScale,float yScale,float zScale)
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]->getSceneObjectID()==objectID)
            _allObjects[i]->adjustForScaling(xScale,yScale,zScale);
    }
}

void CDrawingContainer::removeAllObjects()
{
    for (size_t i=0;i<_allObjects.size();i++)
        delete _allObjects[i];
    _allObjects.clear();
}

void CDrawingContainer::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<_allObjects.size())
    {
        if (_allObjects[i]->announceObjectWillBeErased(objID))
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
        }
        else
            i++;
    }
}

void CDrawingContainer::announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    size_t i=0;
    while (i<_allObjects.size())
    {
        if (_allObjects[i]->announceScriptStateWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript))
        {
            delete _allObjects[i];
            _allObjects.erase(_allObjects.begin()+i);
        }
        else
            i++;
    }
}

void CDrawingContainer::renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(false,false,displayAttrib,renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(false,true,displayAttrib,renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib)
{
    drawAll(true,true,displayAttrib,renderingObject->getFullCumulativeTransformation().getMatrix());
}

void CDrawingContainer::drawAll(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    EASYLOCK(_objectMutex);
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->draw(overlay,transparentObject,displayAttrib,cameraCTM);
}

void CDrawingContainer::drawObjectsParentedWith(bool overlay,bool transparentObject,int parentObjectID,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    if ((displayAttrib&sim_displayattribute_nodrawingobjects)==0)
    {
        EASYLOCK(_objectMutex);
        for (size_t i=0;i<_allObjects.size();i++)
        {
            if ( (_allObjects[i]->getSceneObjectID()==parentObjectID)&&((_allObjects[i]->getObjectType()&sim_drawing_painttag)!=0) )
                _allObjects[i]->draw(overlay,transparentObject,displayAttrib,cameraCTM);
        }
    }
}
