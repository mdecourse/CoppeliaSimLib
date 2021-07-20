#pragma once

#include "_collisionObject_old.h"

class CCollisionObject_old : public _CCollisionObject_old
{
public:
    CCollisionObject_old();
    CCollisionObject_old(int entity1Handle,int entity2Handle);
    virtual ~CCollisionObject_old();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject(bool localReferencesToItOnly);

    void displayCollisionContour();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool isSame(int entity1Handle,int entity2Handle) const;
    void clearCollisionResult();
    bool handleCollision();
    int readCollision(int collObjHandles[2]) const;
    bool getCollisionResult() const;
    bool isCollisionResultValid() const;
    float getCalculationTime() const;
    std::string getUniquePersistentIdString() const;
    void _clearCollisionResult();
    void _setCollisionResult(bool result,int calcTime,int obj1Handle,int obj2Handle,const std::vector<float>& intersect);

    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void performCollectionLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    bool announceCollectionWillBeErased(int collectionHandle,bool copyBuffer);
    CCollisionObject_old* copyYourself();
    int getCollisionColor(int entityHandle) const;
    bool canComputeCollisionContour() const;
    std::string getObjectDescriptiveName() const;

    // Overridden from _CCollisionObject_:
    bool setObjectName(const char* newName,bool check);
    bool setContourWidth(int w);

private:
    void _commonInit();

    std::string _uniquePersistentIdString;

    bool _collisionResult;
    bool _collisionResultValid;
    int _collObjectHandles[2];
    int _calcTimeInMs;
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;

};
