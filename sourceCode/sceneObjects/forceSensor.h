
#pragma once

#include "sceneObject.h"

class CForceSensor : public CSceneObject  
{
public:
    CForceSensor();
    virtual ~CForceSensor();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    void commonInit();

    // Dynamic routines:
    void setDynamicSecondPartIsValid(bool v);
    bool getDynamicSecondPartIsValid() const;
    void setDynamicSecondPartLocalTransform(const C7Vector& tr);

    void addCumulativeForcesAndTorques(const C3Vector& f,const C3Vector& t,int countForAverage);
    void setForceAndTorqueNotValid();

    bool getDynamicForces(C3Vector& f,bool dynamicStepValue) const;
    bool getDynamicTorques(C3Vector& t,bool dynamicStepValue) const;

    void setForceSensorIsBroken();
    bool getForceSensorIsBroken() const;
    float getDynamicPositionError() const;
    float getDynamicOrientationError() const;
    void getDynamicErrorsFull(C3Vector& linear,C3Vector& angular) const;

    bool getStillAutomaticallyBreaking();
    void setForceThreshold(float t);
    float getForceThreshold() const;
    void setTorqueThreshold(float t);
    float getTorqueThreshold() const;
    void setEnableForceThreshold(bool e);
    bool getEnableForceThreshold() const;
    void setEnableTorqueThreshold(bool e);
    bool getEnableTorqueThreshold() const;
    void setConsecutiveThresholdViolationsForBreaking(int count);
    int getConsecutiveThresholdViolationsForBreaking() const;
    C7Vector getDynamicSecondPartLocalTransform() const;

    void setValueCountForFilter(int c);
    int getValueCountForFilter() const;
    void setFilterType(int t);
    int getFilterType() const;

    // Various
    void setSize(float s);
    float getSize() const;

    CColorObject* getColor(bool part2);

protected:
    void _computeFilteredValues();
    void _handleSensorBreaking();

    bool _dynamicSecondPartIsValid;
    float _forceThreshold;
    float _torqueThreshold;
    int _valueCountForFilter;
    int _filterType; //0=average, 1=median
    bool _forceThresholdEnabled;
    bool _torqueThresholdEnabled;
    bool _stillAutomaticallyBreaking;

    bool _forceSensorIsBroken; // if false, then _unbrokenConstraintState is applied, else _brokenConstraintState is applied

    int _consecutiveThresholdViolationsForBreaking;
    int _currentThresholdViolationCount;
    C7Vector _dynamicSecondPartLocalTransform;

    // Variables which need to be serialized & copied
    // Visual attributes:
    float _forceSensorSize;
    CColorObject colorPart1;
    CColorObject colorPart2;

    // Dynamic values:
    std::vector<C3Vector> _cumulatedForces;
    std::vector<C3Vector> _cumulatedTorques;

    C3Vector _cumulativeForcesTmp;
    C3Vector _cumulativeTorquesTmp;

    // Following are forces/torques acquired during a single dyn. calculation step:
    C3Vector _lastForce_dynStep;
    C3Vector _lastTorque_dynStep;
    bool _lastForceAndTorqueValid_dynStep;

    C3Vector _filteredDynamicForces; // do not serialize! (but initialize appropriately)
    C3Vector _filteredDynamicTorques; // do not serialize! (but initialize appropriately)
    bool _filteredValuesAreValid;
};
