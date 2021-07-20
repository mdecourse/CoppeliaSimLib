#pragma once

#include "colorObject.h"
#include "ser.h"
#include "3Vector.h"
#include "7Vector.h"
#include "dynMaterialObject.h"

class CViewableBase;
class CShape;
class CMesh;

class CMeshWrapper
{
public:

    CMeshWrapper();
    virtual ~CMeshWrapper();

    virtual void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    virtual void performSceneObjectLoadingMapping(const std::vector<int>* map);
    virtual void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    virtual void announceSceneObjectWillBeErased(int objectID);
    virtual void setTextureDependencies(int shapeID);
    virtual bool getContainsTransparentComponents();
    virtual void displayGhost(CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors);
    virtual void display(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
    virtual void display_extRenderer(CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex);
    virtual void display_colorCoded(CShape* geomData,int objectId,int displayAttrib);
    virtual CMeshWrapper* copyYourself();
    virtual void scale(float xVal,float yVal,float zVal);
    virtual void setPurePrimitiveType(int theType,float xOrDiameter,float y,float zOrHeight);
    virtual int getPurePrimitiveType();
    virtual bool isMesh();
    virtual bool isPure();
    virtual bool isConvex();
    virtual bool checkIfConvex();
    virtual void setConvex(bool convex);
    virtual bool containsOnlyPureConvexShapes();
    virtual void getCumulativeMeshes(std::vector<float>& vertices,std::vector<int>* indices,std::vector<float>* normals);
    virtual void setColor(const char* colorName,int colorComponent,const float* rgbData,int& rgbDataOffset);
    virtual bool getColor(const char* colorName,int colorComponent,float* rgbData,int& rgbDataOffset);
    virtual void getAllShapeComponentsCumulative(std::vector<CMesh*>& shapeComponentList); // needed by the dynamics routine
    virtual CMesh* getShapeComponentAtIndex(int& index);
    virtual int getComponentCount() const;
    virtual void serialize(CSer& ar,const char* shapeName);
    virtual void preMultiplyAllVerticeLocalFrames(const C7Vector& preTr);
    virtual void flipFaces();
    virtual float getGouraudShadingAngle();
    virtual void setGouraudShadingAngle(float angle);
    virtual float getEdgeThresholdAngle();
    virtual void setEdgeThresholdAngle(float angle);
    virtual void setHideEdgeBorders(bool v);
    virtual bool getHideEdgeBorders();
    virtual int getTextureCount();
    virtual bool hasTextureThatUsesFixedTextureCoordinates();
    virtual void removeAllTextures();
    virtual void getColorStrings(std::string& colorStrings);

    void serializeWrapperInfos(CSer& ar,const char* shapeName);
    void scaleWrapperInfos(float xVal,float yVal,float zVal);
    void scaleMassAndInertia(float xVal,float yVal,float zVal);
    void copyWrapperInfos(CMeshWrapper* target);
    void setDefaultInertiaParams();
    void setMass(float m);
    float getMass();
    void setName(std::string newName);
    std::string getName();

    int getDynMaterialId_old();
    void setDynMaterialId_old(int id);
    // ---------------------


    C7Vector getLocalInertiaFrame();
    void setLocalInertiaFrame(const C7Vector& li);
    C3Vector getPrincipalMomentsOfInertia();
    void setPrincipalMomentsOfInertia(const C3Vector& inertia);

    C7Vector getTransformationsSinceGrouping();
    void setTransformationsSinceGrouping(const C7Vector& tr);

    static void findPrincipalMomentOfInertia(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments);
    static C3X3Matrix getNewTensor(const C3Vector& principalMoments,const C7Vector& newFrame);

    std::vector<CMeshWrapper*> childList;

protected:
    static float _getTensorNonDiagonalMeasure(const C3X3Matrix& tensor);

    std::string _name;

    float _mass;

    int _dynMaterialId_old;

    C7Vector _localInertiaFrame; // frame relative to the shape.
    C3Vector _principalMomentsOfInertia; // remember that we always work with a massless tensor. The tensor is multiplied with the mass in the dynamics module!

    C7Vector _transformationsSinceGrouping; // used to keep track of this geomWrap or geometric's configuration relative to the shape

    bool _convex;

};
