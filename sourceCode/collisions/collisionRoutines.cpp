#include "collisionRoutines.h"
#include "distanceRoutines.h"
#include "pluginContainer.h"
#include "app.h"

//---------------------------- GENERAL COLLISION QUERIES ---------------------------

bool CCollisionRoutine::doEntitiesCollide(int entity1ID,int entity2ID,std::vector<float>* intersections,bool overrideCollidableFlagIfObject1,bool overrideCollidableFlagIfObject2,int collidingObjectIDs[2])
{   // if entity2ID==-1, then all collidable objects are tested against entity1
    // We first check if objects are valid:
    CSceneObject* object1=App::currentWorld->sceneObjects->getObjectFromHandle(entity1ID);
    CSceneObject* object2=App::currentWorld->sceneObjects->getObjectFromHandle(entity2ID);
    CCollection* collection1=nullptr;
    CCollection* collection2=nullptr;
    if (object1==nullptr)
    {
        collection1=App::currentWorld->collections->getObjectFromHandle(entity1ID);
        if (collection1==nullptr)
            return(false);
    }
    if (object2==nullptr)
    {
        collection2=App::currentWorld->collections->getObjectFromHandle(entity2ID);
        if ( (collection2==nullptr)&&(entity2ID!=-1) )
            return(false);
    }
    bool collisionResult=false;
    if (object1!=nullptr)
    { // Here we have an object against..
        if (object2!=nullptr)
        { //...another object
            collisionResult=_doesObjectCollideWithObject(object1,object2,overrideCollidableFlagIfObject1,overrideCollidableFlagIfObject2,intersections);
            if (collisionResult&&(collidingObjectIDs!=nullptr))
            {
                collidingObjectIDs[0]=object1->getObjectHandle();
                collidingObjectIDs[1]=object2->getObjectHandle();
            }
        }
        else
        { // an objects VS a collection or all other objects
            std::vector<CSceneObject*> group;
            if (entity2ID==-1)
            { // Special group here (all objects except the shape):
                std::vector<CSceneObject*> exception;
                exception.push_back(object1);
                App::currentWorld->sceneObjects->getAllCollidableObjectsFromSceneExcept(&exception,group);
            }
            else
            { // Regular group here:
                App::currentWorld->collections->getCollidableObjectsFromCollection(entity2ID,group);
            }

            if (group.size()!=0)
            {
                int collidingGroupObject=-1;
                if (object1->getObjectType()==sim_object_shape_type)
                    collisionResult=_doesGroupCollideWithShape(group,(CShape*)object1,intersections,overrideCollidableFlagIfObject1,collidingGroupObject);
                if (object1->getObjectType()==sim_object_octree_type)
                    collisionResult=_doesGroupCollideWithOctree(group,(COctree*)object1,overrideCollidableFlagIfObject1,collidingGroupObject);
                if (object1->getObjectType()==sim_object_dummy_type)
                    collisionResult=_doesGroupCollideWithDummy(group,(CDummy*)object1,overrideCollidableFlagIfObject1,collidingGroupObject);
                if (object1->getObjectType()==sim_object_pointcloud_type)
                    collisionResult=_doesGroupCollideWithPointCloud(group,(CPointCloud*)object1,overrideCollidableFlagIfObject1,collidingGroupObject);

                if (collisionResult&&(collidingObjectIDs!=nullptr))
                {
                    collidingObjectIDs[0]=object1->getObjectHandle();
                    collidingObjectIDs[1]=collidingGroupObject;
                }
            }
        }
    }
    else
    { // Here we have a group against...
        std::vector<CSceneObject*> group1;
        App::currentWorld->collections->getCollidableObjectsFromCollection(entity1ID,group1);
        if (group1.size()!=0)
        {
            if (object2!=nullptr)
            { // ...an object
                int collidingGroupObject=-1;
                if (object2->getObjectType()==sim_object_shape_type)
                    collisionResult=_doesGroupCollideWithShape(group1,(CShape*)object2,intersections,overrideCollidableFlagIfObject2,collidingGroupObject);
                if (object2->getObjectType()==sim_object_octree_type)
                    collisionResult=_doesGroupCollideWithOctree(group1,(COctree*)object2,overrideCollidableFlagIfObject2,collidingGroupObject);
                if (object2->getObjectType()==sim_object_dummy_type)
                    collisionResult=_doesGroupCollideWithDummy(group1,(CDummy*)object2,overrideCollidableFlagIfObject2,collidingGroupObject);
                if (object2->getObjectType()==sim_object_pointcloud_type)
                    collisionResult=_doesGroupCollideWithPointCloud(group1,(CPointCloud*)object2,overrideCollidableFlagIfObject2,collidingGroupObject);

                if (collisionResult&&(collidingObjectIDs!=nullptr))
                {
                    collidingObjectIDs[0]=collidingGroupObject;
                    collidingObjectIDs[1]=object2->getObjectHandle();
                }
            }
            else
            { // ...another group (or all other objects) (entity2ID could be -1)
                std::vector<CSceneObject*> group2;
                if (entity2ID==-1)
                { // Special group here
                    App::currentWorld->sceneObjects->getAllCollidableObjectsFromSceneExcept(&group1,group2);
                }
                else
                { // Regular group here:
                    App::currentWorld->collections->getCollidableObjectsFromCollection(entity2ID,group2);
                }
                if (group2.size()!=0)
                {
                    int collidingGroupObjects[2]={-1,-1};

                    if (entity1ID!=entity2ID)
                        collisionResult=_doesGroupCollideWithGroup(group1,group2,intersections,collidingGroupObjects);
                    else
                        collisionResult=_doesGroupCollideWithItself(group1,intersections,collidingGroupObjects);

                    if (collisionResult&&(collidingObjectIDs!=nullptr))
                    {
                        collidingObjectIDs[0]=collidingGroupObjects[0];
                        collidingObjectIDs[1]=collidingGroupObjects[1];
                    }
                }
            }
        }
    }
    return(collisionResult);
}

//----------------------------------------------------------------------------------

bool CCollisionRoutine::_doesShapeCollideWithShape(CShape* shape1,CShape* shape2,std::vector<float>* intersections,bool overrideShape1CollidableFlag,bool overrideShape2CollidableFlag)
{   // if intersections is different from nullptr we check for all collisions and
    // append intersection segments to the vector.
    // We never check a shape against itself!!
    if (shape1==shape2)
        return(false);
    if ( ( (shape1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideShape1CollidableFlag) )
        return(false);
    if ( ( (shape2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideShape2CollidableFlag) )
        return(false);

    // Before building collision nodes, check if the shape's bounding boxes collide (new since 9/7/2014):
    if ( (!shape1->isMeshCalculationStructureInitialized())||(!shape2->isMeshCalculationStructureInitialized()) )
    {
        if (!CPluginContainer::geomPlugin_getBoxBoxCollision(shape1->getFullCumulativeTransformation(),shape1->getBoundingBoxHalfSizes(),shape2->getFullCumulativeTransformation(),shape2->getBoundingBoxHalfSizes(),true))
            return(false);
    }

    // Build the collision nodes only when needed. So do it right here!
    shape1->initializeMeshCalculationStructureIfNeeded();
    shape2->initializeMeshCalculationStructureIfNeeded();

    return(shape1->doesShapeCollideWithShape(shape2,intersections));
}

bool CCollisionRoutine::_doesGroupCollideWithShape(const std::vector<CSceneObject*>& group, CShape* shape,std::vector<float>* intersections,bool overrideShapeCollidableFlag,int &collidingGroupObject)
{   // if intersections is different from nullptr we check for ALL shape-shape collisions and
    // append intersection segments to the vector.
    bool returnValue=false;
    for (size_t i=0;i<group.size();i++)
    {
        if (group[i]->getObjectType()==sim_object_shape_type)
        {
            if (shape!=group[i])
            { // never self-collision
                if (_doesShapeCollideWithShape(shape,(CShape*)group[i],intersections,overrideShapeCollidableFlag,true))
                {
                    returnValue=true;
                    collidingGroupObject=group[i]->getObjectHandle();
                    if (intersections==nullptr)
                        return(true);
                }
            }
        }
        if (group[i]->getObjectType()==sim_object_octree_type)
        {
            if (!returnValue)
            {
                if (_doesOctreeCollideWithShape((COctree*)group[i],shape,true,overrideShapeCollidableFlag))
                {
                    returnValue=true;
                    collidingGroupObject=group[i]->getObjectHandle();
                    if (intersections==nullptr)
                        return(true);
                }
            }
        }
    }
    return(returnValue);
}

bool CCollisionRoutine::_doesGroupCollideWithGroup(const std::vector<CSceneObject*>& group1,const std::vector<CSceneObject*>& group2,std::vector<float>* intersections,int collidingGroupObjects[2])
{   // if intersections is different from nullptr we check for all collisions and
    // append intersection segments to the vector.
    bool returnValue=false;
    std::vector<CSceneObject*> checkedPairs;
    for (size_t i=0;i<group1.size();i++)
    {
        for (size_t j=0;j<group2.size();j++)
        {
            if (group1[i]!=group2[j])
            { // never check an object against itself
                // Never check twice the same pair:
                bool present=false;
                for (size_t k=0;k<checkedPairs.size()/2;k++)
                {
                    if ((group2[j]==checkedPairs[2*k+0])&&(group1[i]==checkedPairs[2*k+1]))
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                {
                    checkedPairs.push_back(group1[i]);
                    checkedPairs.push_back(group2[j]);

                    bool doIt=(!returnValue);
                    if ( (!doIt)&&(intersections!=nullptr) )
                    { // we still might have to do it if we have shape-shape colldetection (for the contour)
                        doIt=(group1[i]->getObjectType()==sim_object_shape_type)&&(group2[j]->getObjectType()==sim_object_shape_type);
                    }

                    if (doIt)
                    {
                        if (_doesObjectCollideWithObject(group1[i],group2[j],true,true,intersections))
                        {
                            collidingGroupObjects[0]=group1[i]->getObjectHandle();
                            collidingGroupObjects[1]=group2[j]->getObjectHandle();
                            if (intersections==nullptr)
                                return(true);
                            returnValue=true;
                        }
                    }
                }
            }
        }
    }
    return(returnValue);
}

bool CCollisionRoutine::_areObjectBoundingBoxesOverlapping(CSceneObject* obj1,CSceneObject* obj2)
{
    CSceneObject* objs[2]={obj1,obj2};
    C3Vector halfSizes[2];
    C7Vector m[2];
    for (size_t cnt=0;cnt<2;cnt++)
    {
        CSceneObject* obj=objs[cnt];
        if (obj->getObjectType()==sim_object_shape_type)
        {
            halfSizes[cnt]=((CShape*)obj)->getBoundingBoxHalfSizes();
            m[cnt]=obj->getFullCumulativeTransformation();
        }
        if (obj->getObjectType()==sim_object_dummy_type)
        {
            halfSizes[cnt]=C3Vector(0.0001f,0.0001f,0.0001f);
            m[cnt]=obj->getFullCumulativeTransformation();
        }
        if (obj->getObjectType()==sim_object_octree_type)
            ((COctree*)obj)->getTransfAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
        if (obj->getObjectType()==sim_object_pointcloud_type)
            ((CPointCloud*)obj)->getTransfAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
    }
    return (CPluginContainer::geomPlugin_getBoxBoxCollision(m[0],halfSizes[0],m[1],halfSizes[1],true));
}

bool CCollisionRoutine::_doesOctreeCollideWithShape(COctree* octree,CShape* shape,bool overrideOctreeCollidableFlag,bool overrideShapeCollidableFlag)
{
    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideOctreeCollidableFlag) )
        return(false);
    if ( ( (shape->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideShapeCollidableFlag) )
        return(false);

    // Before building collision nodes, check if the shape's bounding boxes collide (new since 9/7/2014):
    if (!shape->isMeshCalculationStructureInitialized())
    {
        if (!_areObjectBoundingBoxesOverlapping(octree,shape))
            return(false);
    }

    // Build the collision nodes only when needed. So do it right here!
    shape->initializeMeshCalculationStructureIfNeeded();

    // TODO_CACHING
    int meshCaching=-1;
    unsigned long long int ocCaching=0;
    return(CPluginContainer::geomPlugin_getMeshOctreeCollision(shape->_meshCalculationStructure,shape->getFullCumulativeTransformation(),octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),&meshCaching,&ocCaching));
}

bool CCollisionRoutine::_doesOctreeCollideWithOctree(COctree* octree1,COctree* octree2,bool overrideOctree1CollidableFlag,bool overrideOctree2CollidableFlag)
{
    if (octree1==octree2)
        return(false); // never self-collision
    if (octree1->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if (octree2->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideOctree1CollidableFlag) )
        return(false);
    if ( ( (octree2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideOctree2CollidableFlag) )
        return(false);
    if (!_areObjectBoundingBoxesOverlapping(octree1,octree2))
        return(false);
    return(CPluginContainer::geomPlugin_getOctreeOctreeCollision(octree1->getOctreeInfo(),octree1->getFullCumulativeTransformation().getMatrix(),octree2->getOctreeInfo(),octree2->getFullCumulativeTransformation().getMatrix()));
}

bool CCollisionRoutine::_doesOctreeCollideWithPointCloud(COctree* octree,CPointCloud* pointCloud,bool overrideOctreeCollidableFlag,bool overridePointCloudCollidableFlag)
{
    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if (pointCloud->getPointCloudInfo()==nullptr)
        return(false); // PointCloud is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideOctreeCollidableFlag) )
        return(false);
    if ( ( (pointCloud->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overridePointCloudCollidableFlag) )
        return(false);

    if (!_areObjectBoundingBoxesOverlapping(octree,pointCloud))
        return(false);

    // TODO_CACHING
    unsigned long long int ocCaching=0;
    unsigned long long int pcCaching=0;
    return(CPluginContainer::geomPlugin_getOctreePtcloudCollision(octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),pointCloud->getPointCloudInfo(),pointCloud->getFullCumulativeTransformation(),&ocCaching,&pcCaching));
}

bool CCollisionRoutine::_doesOctreeCollideWithDummy(COctree* octree,CDummy* dummy,bool overrideOctreeCollidableFlag,bool overrideDummyCollidableFlag)
{
    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideOctreeCollidableFlag) )
        return(false);
    if ( ( (dummy->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0 )&&(!overrideDummyCollidableFlag) )
        return(false);

    // TODO_CACHING
    unsigned long long int ocCaching=0;
    return(CPluginContainer::geomPlugin_getOctreePointCollision(octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),dummy->getFullCumulativeTransformation().X,nullptr,&ocCaching));
}

bool CCollisionRoutine::_doesObjectCollideWithObject(CSceneObject* object1,CSceneObject* object2,bool overrideObject1CollidableFlag,bool overrideObject2CollidableFlag,std::vector<float>* intersections)
{
    if (object1->getObjectType()==sim_object_shape_type)
    {
        if (object2->getObjectType()==sim_object_shape_type)
            return(_doesShapeCollideWithShape((CShape*)object1,(CShape*)object2,intersections,overrideObject1CollidableFlag,overrideObject2CollidableFlag));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_doesOctreeCollideWithShape((COctree*)object2,(CShape*)object1,overrideObject2CollidableFlag,overrideObject1CollidableFlag));
    }
    if (object1->getObjectType()==sim_object_octree_type)
    {
        if (object2->getObjectType()==sim_object_shape_type)
            return(_doesOctreeCollideWithShape((COctree*)object1,(CShape*)object2,overrideObject1CollidableFlag,overrideObject2CollidableFlag));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_doesOctreeCollideWithOctree((COctree*)object1,(COctree*)object2,overrideObject1CollidableFlag,overrideObject2CollidableFlag));
        if (object2->getObjectType()==sim_object_dummy_type)
            return(_doesOctreeCollideWithDummy((COctree*)object1,(CDummy*)object2,overrideObject1CollidableFlag,overrideObject2CollidableFlag));
        if (object2->getObjectType()==sim_object_pointcloud_type)
            return(_doesOctreeCollideWithPointCloud((COctree*)object1,(CPointCloud*)object2,overrideObject1CollidableFlag,overrideObject2CollidableFlag));
    }
    if (object1->getObjectType()==sim_object_dummy_type)
    {
        if (object2->getObjectType()==sim_object_octree_type)
            return(_doesOctreeCollideWithDummy((COctree*)object2,(CDummy*)object1,overrideObject2CollidableFlag,overrideObject1CollidableFlag));
    }
    if (object1->getObjectType()==sim_object_pointcloud_type)
    {
        if (object2->getObjectType()==sim_object_octree_type)
            return(_doesOctreeCollideWithPointCloud((COctree*)object2,(CPointCloud*)object1,overrideObject2CollidableFlag,overrideObject1CollidableFlag));
    }
    return(false);
}


bool CCollisionRoutine::_doesGroupCollideWithOctree(const std::vector<CSceneObject*>& group,COctree* octree,bool overrideOctreeCollidableFlag,int& collidingGroupObject)
{
    for (size_t i=0;i<group.size();i++)
    {
        if (group[i]->getObjectType()==sim_object_shape_type)
        {
            if (_doesOctreeCollideWithShape(octree,(CShape*)group[i],overrideOctreeCollidableFlag,true))
            {
                collidingGroupObject=group[i]->getObjectHandle();
                return(true);
            }
        }
        if (group[i]->getObjectType()==sim_object_dummy_type)
        {
            if (_doesOctreeCollideWithDummy(octree,(CDummy*)group[i],overrideOctreeCollidableFlag,true))
            {
                collidingGroupObject=group[i]->getObjectHandle();
                return(true);
            }
        }
        if (group[i]->getObjectType()==sim_object_octree_type)
        {
            if (octree!=group[i])
            { // never self-collision
                if (_doesOctreeCollideWithOctree(octree,(COctree*)group[i],overrideOctreeCollidableFlag,true))
                {
                    collidingGroupObject=group[i]->getObjectHandle();
                    return(true);
                }
            }
        }
        if (group[i]->getObjectType()==sim_object_pointcloud_type)
        {
            if (_doesOctreeCollideWithPointCloud(octree,(CPointCloud*)group[i],overrideOctreeCollidableFlag,true))
            {
                collidingGroupObject=group[i]->getObjectHandle();
                return(true);
            }
        }
    }
    return(false);
}

bool CCollisionRoutine::_doesGroupCollideWithDummy(const std::vector<CSceneObject*>& group,CDummy* dummy,bool overrideDummyCollidableFlag,int& collidingGroupObject)
{
    for (size_t i=0;i<group.size();i++)
    {
        if (group[i]->getObjectType()==sim_object_octree_type)
        {
            if (_doesOctreeCollideWithDummy((COctree*)group[i],dummy,overrideDummyCollidableFlag,true))
            {
                collidingGroupObject=group[i]->getObjectHandle();
                return(true);
            }
        }
    }
    return(false);
}

bool CCollisionRoutine::_doesGroupCollideWithPointCloud(const std::vector<CSceneObject*>& group,CPointCloud* pointCloud,bool overridePointCloudCollidableFlag,int& collidingGroupObject)
{
    for (size_t i=0;i<group.size();i++)
    {
        if (group[i]->getObjectType()==sim_object_octree_type)
        {
            if (_doesOctreeCollideWithPointCloud((COctree*)group[i],pointCloud,overridePointCloudCollidableFlag,true))
            {
                collidingGroupObject=group[i]->getObjectHandle();
                return(true);
            }
        }
    }
    return(false);
}

bool CCollisionRoutine::_doesGroupCollideWithItself(const std::vector<CSceneObject*>& group,std::vector<float>* intersections,int collidingGroupObjects[2])
{   // if intersections is different from nullptr we check for all collisions and
    // append intersection segments to the vector.

    std::vector<CSceneObject*> objPairs; // Object pairs we need to check
    for (size_t i=0;i<group.size();i++)
    {
        CSceneObject* obj1=group[i];
        int csci1=obj1->getCollectionSelfCollisionIndicator();
        for (size_t j=0;j<group.size();j++)
        {
            CSceneObject* obj2=group[j];
            int csci2=obj2->getCollectionSelfCollisionIndicator();
            if (obj1!=obj2)
            { // We never check an object against itself!
                if (abs(csci1-csci2)!=1)
                { // the collection self collision indicators differences is not 1
                    // We now check if these partners are already present in objPairs
                    bool absent=true;
                    for (size_t k=0;k<objPairs.size()/2;k++)
                    {
                        if ( ( (objPairs[2*k+0]==obj1)&&(objPairs[2*k+1]==obj2) ) || ( (objPairs[2*k+0]==obj2)&&(objPairs[2*k+1]==obj1) ) )
                        {
                            absent=false;
                            break;
                        }
                    }
                    if (absent)
                    {
                        objPairs.push_back(obj1);
                        objPairs.push_back(obj2);
                    }
                }
            }
        }
    }

    // Here we check all objects from the two groups against each other
    bool returnValue=false;
    for (size_t i=0;i<objPairs.size()/2;i++)
    {
        CSceneObject* obj1=objPairs[2*i+0];
        CSceneObject* obj2=objPairs[2*i+1];
        if (obj1!=nullptr) // to take into account a removed pair
        {
            bool doIt=(!returnValue);
            if ( (!doIt)&&(intersections!=nullptr) )
            { // we still might have to do it if we have shape-shape colldetection (for the contour)
                doIt=(obj1->getObjectType()==sim_object_shape_type)&&(obj2->getObjectType()==sim_object_shape_type);
            }
            if (doIt)
            {
                if (_doesObjectCollideWithObject(obj1,obj2,true,true,intersections))
                {
                    collidingGroupObjects[0]=obj1->getObjectHandle();
                    collidingGroupObjects[1]=obj2->getObjectHandle();
                    if (intersections==nullptr)
                        return(true);
                    returnValue=true;
                }
            }
        }
    }
    return(returnValue);
}
