
#include "multishapeEditMode.h"
#include "mesh.h"

CMultishapeEditMode::CMultishapeEditMode(CShape* shape)
{
    _shape=shape;
    _shape->getMeshWrapper()->getAllShapeComponentsCumulative(_multishapeGeometricComponents);
    _multishapeGeometricComponentIndex=-1;
}

CMultishapeEditMode::~CMultishapeEditMode()
{
}

bool CMultishapeEditMode::processCommand(int commandID)
{ // Return value is true means success
    return(false);
}

CShape* CMultishapeEditMode::getEditModeMultishape()
{
    return(_shape);
}

int CMultishapeEditMode::getMultishapeGeometricComponentsSize()
{
    return(int(_multishapeGeometricComponents.size()));
}

int CMultishapeEditMode::getMultishapeGeometricComponentIndex()
{
    return(_multishapeGeometricComponentIndex);
}

void CMultishapeEditMode::setMultishapeGeometricComponentIndex(int index)
{
    _multishapeGeometricComponentIndex=index;
}

bool CMultishapeEditMode::isCurrentMultishapeGeometricComponentValid()
{
    return((_multishapeGeometricComponentIndex>=0)&&(_multishapeGeometricComponentIndex<int(_multishapeGeometricComponents.size())));
}

CMesh* CMultishapeEditMode::getCurrentMultishapeGeometricComponent()
{
    return(_multishapeGeometricComponents[_multishapeGeometricComponentIndex]);
}

CMesh* CMultishapeEditMode::getMultishapeGeometricComponentAtIndex(int index)
{
    return(_multishapeGeometricComponents[index]);
}

void CMultishapeEditMode::displayAllGeometricComponents(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling)
{
    for (int i=0;i<int(_multishapeGeometricComponents.size());i++)
        _multishapeGeometricComponents[i]->display(geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,i==_multishapeGeometricComponentIndex);
}
