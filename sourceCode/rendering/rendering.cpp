#include "rendering.h"

#ifdef SIM_WITH_OPENGL
#include "embeddedFonts.h"
#include "vDateTime.h"
#include "glBufferObjects.h"
#include "oglExt.h"

CGlBufferObjects* _glBufferObjects=nullptr;
bool _glInitialized=false;

void initializeRendering()
{
    _glBufferObjects=new CGlBufferObjects();
}

void deinitializeRendering()
{
    delete _glBufferObjects;
    _glBufferObjects=nullptr;
}

void initGl_ifNeeded()
{ // when calling this we need to have a valid openGl context!!
    if (_glInitialized)
        return;
    _glInitialized=true;

    oglExt::prepareExtensionFunctions(App::userSettings->forceFboViaExt);

    // Turn off vertical screen synchronization. Very important, otherwise the simulation gets drastically slowed down!
    // Check these threads:
    // http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=271567
    oglExt::turnOffVSync(App::userSettings->vsync);

    //CIloIlo::nonPowerOfTwoTexturesAvailable=oglExt::areNonPowerOfTwoTexturesAvailable();

    oglExt::initDefaultGlValues();

    int lights;
    glGetIntegerv(GL_MAX_LIGHTS,&lights);
    CLight::setMaxAvailableOglLights(lights);

    if (App::sc>1)// (userSettings->highResDisplay==1)||(highResDisplay&&(userSettings->highResDisplay==-1)) )
    {
        ogl::loadBitmapFont(SIMFONT_MS_SANS_SERIF_30,32,0);
        ogl::loadBitmapFont(SIMFONT_LUCIDA_CONSOLE_26,32,1);
    }
    else
    {
        ogl::loadBitmapFont(SIMFONT_MS_SANS_SERIF,16,0);
        ogl::loadBitmapFont(SIMFONT_COURIER_NEW,16,1);
    }

    ogl::loadOutlineFont(SIMOUTLINEFONT_ARIAL_INT,SIMOUTLINEFONT_ARIAL_FLOAT);

    std::string glVer("OpenGL: ");
    std::string tmp="(none given)";
    if (glGetString(GL_VENDOR)!=nullptr)
        tmp=(char*)glGetString(GL_VENDOR);
    glVer+=tmp.c_str();
    glVer+=", Renderer: ";
    tmp="(none given)";
    if (glGetString(GL_RENDERER)!=nullptr)
        tmp=(char*)glGetString(GL_RENDERER);
    glVer+=tmp.c_str();
    glVer+=", Version: ";
    tmp="(none given)";
    if (glGetString(GL_VERSION)!=nullptr)
        tmp=(char*)glGetString(GL_VERSION);
    glVer+=tmp.c_str();
    App::logMsg(sim_verbosity_loadinfos,glVer.c_str());
}

void deinitGl_ifNeeded()
{
    _glInitialized=false;
}

void increaseVertexBufferRefCnt(int vertexBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->increaseVertexBufferRefCnt(vertexBufferId);
}

void decreaseVertexBufferRefCnt(int vertexBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->removeVertexBuffer(vertexBufferId);
}

void increaseNormalBufferRefCnt(int normalBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->increaseNormalBufferRefCnt(normalBufferId);
}

void decreaseNormalBufferRefCnt(int normalBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->removeNormalBuffer(normalBufferId);
}

void increaseEdgeBufferRefCnt(int edgeBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->increaseEdgeBufferRefCnt(edgeBufferId);
}

void decreaseEdgeBufferRefCnt(int edgeBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->removeEdgeBuffer(edgeBufferId);
}

void decreaseTexCoordBufferRefCnt(int texCoordBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->removeTexCoordBuffer(texCoordBufferId);
}

void _drawTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,const float* textureCoords,int* vertexBufferId,int* normalBufferId,int* texCoordBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->drawTriangles(vertices,verticesCnt,indices,indicesCnt,normals,textureCoords,vertexBufferId,normalBufferId,texCoordBufferId);
}

bool _drawEdges(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const unsigned char* edges,int* edgeBufferId)
{
    if (_glBufferObjects!=nullptr)
        return(_glBufferObjects->drawEdges(vertices,verticesCnt,indices,indicesCnt,edges,edgeBufferId));
    return(false);
}

void _drawColorCodedTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,int* vertexBufferId,int* normalBufferId)
{
    if (_glBufferObjects!=nullptr)
        _glBufferObjects->drawColorCodedTriangles(vertices,verticesCnt,indices,indicesCnt,normals,vertexBufferId,normalBufferId);
}

void makeColorCurrent(const CColorObject* visParam,bool forceNonTransparent,bool useAuxiliaryComponent)
{
    if (useAuxiliaryComponent)
    { // temperature, etc. colors:
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,visParam->getColorsPtr()+12);
        ogl::setBlending(false);
    }
    else
    { // regular colors:
        if (visParam->getFlash())
        {
            float t=0.0f;
            if (visParam->getUseSimulationTime()&&(!App::currentWorld->simulation->isSimulationStopped()))
                t=float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f;
            if (!visParam->getUseSimulationTime())
                t=float(VDateTime::getTimeInMs())/1000.0f;
            t+=visParam->getFlashPhase()/visParam->getFlashFrequency();
            t=CMath::robustFmod(t,1.0f/visParam->getFlashFrequency())*visParam->getFlashFrequency();
            if (t>(1.0f-visParam->getFlashRatio()))
            { // Flash is on
                    t=t-1.0f+visParam->getFlashRatio();
                    t/=visParam->getFlashRatio();
                    t=sin(t*piValue_f);
                    float l=0.0f;
                    float col0[12]={visParam->getColorsPtr()[0],visParam->getColorsPtr()[1],visParam->getColorsPtr()[2],visParam->getColorsPtr()[3],visParam->getColorsPtr()[4],visParam->getColorsPtr()[5],visParam->getColorsPtr()[6],visParam->getColorsPtr()[7],visParam->getColorsPtr()[8],0.0f,0.0f,0.0f};
                    float col1[12]={visParam->getColorsPtr()[0]*l,visParam->getColorsPtr()[1]*l,visParam->getColorsPtr()[2]*l,visParam->getColorsPtr()[3]*l,visParam->getColorsPtr()[4]*l,visParam->getColorsPtr()[5]*l,visParam->getColorsPtr()[6]*l,visParam->getColorsPtr()[7]*l,visParam->getColorsPtr()[8]*l,visParam->getColorsPtr()[9],visParam->getColorsPtr()[10],visParam->getColorsPtr()[11]};
                    for (int i=0;i<12;i++)
                        col0[i]=col0[i]*(1.0f-t)+col1[i]*t;
                    ogl::setMaterialColor(col0,col0+6,col0+9);
                    ogl::setShininess(visParam->getShininess());
                    ogl::setAlpha(visParam->getTransparencyFactor());
            }
            else
            { // flash is off
                ogl::setMaterialColor(visParam->getColorsPtr(),visParam->getColorsPtr()+6,ogl::colorBlack);
                ogl::setShininess(visParam->getShininess());
                ogl::setAlpha(visParam->getTransparencyFactor());
            }
        }
        else
        {
            ogl::setMaterialColor(visParam->getColorsPtr(),visParam->getColorsPtr()+6,visParam->getColorsPtr()+9);
            ogl::setShininess(visParam->getShininess());
            ogl::setAlpha(visParam->getTransparencyFactor());
        }

        if (visParam->getTranslucid()&&(!forceNonTransparent))
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        else
            ogl::setBlending(false);
    }
}

void _activateNonAmbientLights(int lightHandle,CViewableBase* viewable)
{ // -2: none, -1: all, otherwise the handle of the light or the collection
    // First deactivate all:
    for (int i=0;i<CLight::getMaxAvailableOglLights();i++)
        glDisable(GL_LIGHT0+i);
    App::currentWorld->environment->setNonAmbientLightsActive(false);

    if (lightHandle>-2)
    {
        App::currentWorld->environment->setNonAmbientLightsActive(true);
        GLfloat lightPos[]={0.0f,0.0f,0.0f,1.0f};
        GLfloat lightDir[3];
        int activeLightCounter=0;
        bool useLocalLights=viewable->getuseLocalLights();

        std::vector<CLight*> lList;
        if (lightHandle==-1)
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getLightCount();i++)
            {
                CLight* light=App::currentWorld->sceneObjects->getLightFromIndex(i);
                lList.push_back(light);
            }
        }
        else
        {
            if (lightHandle<SIM_IDSTART_COLLECTION)
            {
                CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(lightHandle);
                if (light!=nullptr)
                    lList.push_back(light);
            }
            else
            {
                CCollection* gr=App::currentWorld->collections->getObjectFromHandle(lightHandle);
                if (gr!=nullptr)
                {
                    for (size_t i=0;i<gr->getSceneObjectCountInCollection();i++)
                    {
                        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(gr->getSceneObjectHandleFromIndex(i));
                        if (light!=nullptr)
                            lList.push_back(light);
                    }
                }
            }
        }

        for (size_t i=0;i<lList.size();i++)
        {
            CLight* light=lList[i];
            bool LocalLight=light->getLightIsLocal();
            if ( ((!useLocalLights)&&(!LocalLight))||(useLocalLights&&LocalLight&&light->isObjectParentedWith(viewable)) )
            {
                if ((light->getLightActive())&&(activeLightCounter<CLight::getMaxAvailableOglLights()))
                {
                    C7Vector tr(light->getFullCumulativeTransformation());
                    C4X4Matrix m(tr.getMatrix());
                    if (light->getLightType()==sim_light_directional_subtype)
                    {
                        lightPos[0]=-m.M.axis[2](0);
                        lightPos[1]=-m.M.axis[2](1);
                        lightPos[2]=-m.M.axis[2](2);
                        lightPos[3]=0.0f;
                    }
                    else
                    {
                        lightPos[0]=m.X(0);
                        lightPos[1]=m.X(1);
                        lightPos[2]=m.X(2);
                        lightPos[3]=1.0f;
                    }
                    lightDir[0]=m.M.axis[2](0);
                    lightDir[1]=m.M.axis[2](1);
                    lightDir[2]=m.M.axis[2](2);
                    glLightfv(GL_LIGHT0+activeLightCounter,GL_POSITION,lightPos);
                    glLightfv(GL_LIGHT0+activeLightCounter,GL_SPOT_DIRECTION,lightDir);
                    if (light->getLightType()==sim_light_omnidirectional_subtype)
                        glLightf(GL_LIGHT0+activeLightCounter,GL_SPOT_CUTOFF,180.0f);
                    if (light->getLightType()==sim_light_directional_subtype)
                        glLightf(GL_LIGHT0+activeLightCounter,GL_SPOT_CUTOFF,90.0f);
                    if (light->getLightType()==sim_light_spot_subtype)
                    {
                        float coa=light->getSpotCutoffAngle()*radToDeg_f;
                        if (coa>89.0f) // 90.0f causes problems on MacOS!!!
                        coa=89.0f;
                        glLightf(GL_LIGHT0+activeLightCounter,GL_SPOT_CUTOFF,coa);
                    }
                    glLightf(GL_LIGHT0+activeLightCounter,GL_SPOT_EXPONENT,float(light->getSpotExponent())); // glLighti & GL_SPOT_EXPONENT causes problems on MacOS!!!

                    float black[4]={0.0f,0.0f,0.0f,1.0f};

                    glLightfv(GL_LIGHT0+activeLightCounter,GL_AMBIENT,black);
                    if ((viewable->getDisabledColorComponents()&2)==0)
                    {
                        float diffuseLight[4]={light->getColor(true)->getColorsPtr()[3],light->getColor(true)->getColorsPtr()[4],light->getColor(true)->getColorsPtr()[5],1.0f};
                        glLightfv(GL_LIGHT0+activeLightCounter,GL_DIFFUSE,diffuseLight);
                    }
                    else
                        glLightfv(GL_LIGHT0+activeLightCounter,GL_DIFFUSE,black);
                    if ((viewable->getDisabledColorComponents()&4)==0)
                    {
                        float specularLight[4]={light->getColor(true)->getColorsPtr()[6],light->getColor(true)->getColorsPtr()[7],light->getColor(true)->getColorsPtr()[8],1.0f};
                        glLightfv(GL_LIGHT0+activeLightCounter,GL_SPECULAR,specularLight);
                    }
                    else
                        glLightfv(GL_LIGHT0+activeLightCounter,GL_SPECULAR,black);
                    glLightf(GL_LIGHT0+activeLightCounter,GL_CONSTANT_ATTENUATION,
                    light->getAttenuationFactor(CONSTANT_ATTENUATION));
                    glLightf(GL_LIGHT0+activeLightCounter,GL_LINEAR_ATTENUATION,
                    light->getAttenuationFactor(LINEAR_ATTENUATION));
                    glLightf(GL_LIGHT0+activeLightCounter,GL_QUADRATIC_ATTENUATION,
                    light->getAttenuationFactor(QUADRATIC_ATTENUATION));
                    glEnable(GL_LIGHT0+activeLightCounter);
                    activeLightCounter++;
                }
            }
            if (activeLightCounter>=CLight::getMaxAvailableOglLights())
                break;
        }
    }
}

void _prepareAuxClippingPlanes()
{
    _prepareOrEnableAuxClippingPlanes(true,-1);
}

void _enableAuxClippingPlanes(int objID)
{
    _prepareOrEnableAuxClippingPlanes(false,objID);
}

void _prepareOrEnableAuxClippingPlanes(bool prepare,int objID)
{
    if (App::currentWorld->mainSettings->clippingPlanesDisabled)
        return;
    int cpi=0;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getMirrorCount();i++)
    {
        if (cpi<5)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromIndex(i);
            if ( (!it->getIsMirror())&&it->getActive() )
            {
                if (prepare)
                {
                    C7Vector mtr(it->getFullCumulativeTransformation());
                    C3Vector mtrN(mtr.Q.getMatrix().axis[2]);
                    float d=(mtrN*mtr.X);
                    double cpv[4]={-mtrN(0),-mtrN(1),-mtrN(2),d};
                    glClipPlane(GL_CLIP_PLANE1+cpi,cpv);
                }
                else
                {
                    int clipObj=it->getClippingObjectOrCollection();
                    bool clipIt=false;
                    if ( (clipObj>=SIM_IDSTART_COLLECTION)&&(clipObj<=SIM_IDEND_COLLECTION) )
                    { // collection
                        CCollection* coll=App::currentWorld->collections->getObjectFromHandle(clipObj);
                        clipIt=coll->isObjectInCollection(objID);
                    }
                    else
                        clipIt=(clipObj==-1)||(clipObj==objID);
                    if (clipIt)
                        glEnable(GL_CLIP_PLANE1+cpi);
                }
                cpi++;
            }
        }
    }
}

void _disableAuxClippingPlanes()
{
    for (int i=0;i<7;i++)
        glDisable(GL_CLIP_PLANE1+i);
}

void _drawReference(CSceneObject* object,float refSize)
{   // refSize is 0.0f by default --> size depends on the bounding box
    float s;
    if (refSize!=0.0f)
        s=refSize;
    else
    {
        C3Vector minV,maxV;
        if (!object->getMarkingBoundingBox(minV,maxV))
            return; // no bounding box --> no reference!
        maxV-=minV;
        s=(maxV(0)+maxV(1)+maxV(2))/4.0f;
    }
    glPushMatrix();
    ogl::drawReference(s,true,true,true,nullptr);
    glPopMatrix();
}

void _displayBoundingBox(CSceneObject* object,int displayAttrib,bool displRef,float refSize)
{   // displRef is true by default, refSize is 0.0f by default
    if ((displayAttrib&sim_displayattribute_selected)==0)
        return;
    if (!App::userSettings->displayBoundingBoxeWhenObjectSelected)
        return;

    C3Vector bbMin,bbMax;
    bbMin.clear();
    bbMax.clear();
    if (object->getModelBase())
    {
        C7Vector ctmi(object->getCumulativeTransformation().getInverse());
        bool b=true;
        if (!object->getGlobalMarkingBoundingBox(ctmi,bbMin,bbMax,b,true,(displayAttrib&sim_displayattribute_forvisionsensor)==0))
            return; // no boundingbox to display!
        glLineStipple(1,0x0F0F);
        glLineWidth(2.0f);
        glEnable(GL_LINE_STIPPLE);
    }
    else
    {
        if (!object->getMarkingBoundingBox(bbMin,bbMax))
            return; // No bounding box to display!
    }
    C3Vector bbs(bbMax-bbMin);
    // Bounding box is 4% bigger:
    C3Vector dx(bbs(0)*0.02f,bbs(1)*0.02f,bbs(2)*0.02f);
    bool avail=true;
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    if ((displayAttrib&sim_displayattribute_mainselection)&&avail)
    {
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
        avail=false;
    }
    if (((displayAttrib&sim_displayattribute_mainselection)==0)&&avail)
    {
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
        avail=false;
    }

    if (!avail)
    {
        App::currentWorld->environment->temporarilyDeactivateFog();

        ogl::buffer.clear();
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMin(1)-dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMin(1)-dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMin(1)-dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMin(1)-dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMin(1)-dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMax(1)+dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMax(1)+dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMax(1)+dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMax(1)+dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMax(1)+dx(1),bbMax(2)+dx(2));
        ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,nullptr);

        ogl::buffer.clear();
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMin(1)-dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMin(0)-dx(0),bbMax(1)+dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMin(1)-dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMax(1)+dx(1),bbMin(2)-dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMin(1)-dx(1),bbMax(2)+dx(2));
        ogl::addBuffer3DPoints(bbMax(0)+dx(0),bbMax(1)+dx(1),bbMax(2)+dx(2));
        ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
        ogl::buffer.clear();
        App::currentWorld->environment->reactivateFogThatWasTemporarilyDisabled();
        if (true)
        {
            C4Vector r(object->getFullCumulativeTransformation().Q);
            C3Vector absV;
            float maxH=0.0f;
            int highestIndex[3];
            for (int i=0;i<2;i++)
            {
                if (i==0)
                    absV(0)=bbMin(0);
                else
                    absV(0)=bbMax(0);
                for (int j=0;j<2;j++)
                {
                    if (j==0)
                        absV(1)=bbMin(1);
                    else
                        absV(1)=bbMax(1);
                    for (int k=0;k<2;k++)
                    {
                        if (k==0)
                            absV(2)=bbMin(2);
                        else
                            absV(2)=bbMax(2);
                        float h=(r*absV)(2);
                        if (h>(maxH+0.001f)) // added 0.001f to avoid that the label jumps when the box is aligned with the x/y plane
                        {
                            maxH=h;
                            highestIndex[0]=i;
                            highestIndex[1]=j;
                            highestIndex[2]=k;
                        }
                    }
                }
            }

            C3Vector corner;
            C3Vector corner2;
            for (int i=0;i<3;i++)
            {
                if (highestIndex[i]==0)
                    corner(i)=bbMin(i)-dx(i);
                else
                    corner(i)=bbMax(i)+dx(i);
                corner2(i)=corner(i)*(1.1f+0.15f*float((object->getObjectHandle()>>((2-i)*2))%4));
            }
            App::currentWorld->environment->temporarilyDeactivateFog();
            ogl::drawSingle3dLine(corner.data,corner2.data,nullptr);
            ogl::drawBitmapTextTo3dPosition(corner2.data,object->getDisplayName().c_str(),nullptr);
            App::currentWorld->environment->reactivateFogThatWasTemporarilyDisabled();
        }
        glLineWidth(1.0f);
        if (displRef)
            _drawReference(object,refSize);
    }

    glDisable(GL_LINE_STIPPLE);
    avail=true;
}

void _selectLights(CSceneObject* object,CViewableBase* viewable)
{
    object->setRestoreToDefaultLights(false);
    if (App::currentWorld->environment->areNonAmbientLightsActive())
    {
        if (object->getSpecificLight()!=-1)
        {
            _activateNonAmbientLights(object->getSpecificLight(),viewable);
            object->setRestoreToDefaultLights(true);
        }
    }
}

void _restoreDefaultLights(CSceneObject* object,CViewableBase* viewable)
{
    if (object->getRestoreToDefaultLights())
        _activateNonAmbientLights(-1,viewable);
}

void _commonStart(CSceneObject* object,CViewableBase* viewable,int displayAttrib)
{
    _selectLights(object,viewable);
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);

    C7Vector tr=object->getCumulativeTransformation();
    glTranslatef(tr.X(0),tr.X(1),tr.X(2));
    C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
}

void _commonFinish(CSceneObject* object,CViewableBase* viewable)
{
    glPopAttrib();
    glPopMatrix();
    ogl::setBlending(false);
    glDisable(GL_CULL_FACE);
    _restoreDefaultLights(object,viewable);
}

bool _start3DTextureDisplay(CTextureProperty* tp)
{
    int _textureOrVisionSensorObjectID=tp->getTextureObjectID();
    if (_textureOrVisionSensorObjectID==-1)
        return(false);
    CTextureObject* it=nullptr;
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_TEXTURE)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_TEXTURE))
        it=App::currentWorld->textureContainer->getObject(_textureOrVisionSensorObjectID);
    CVisionSensor* rs=nullptr;
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_SCENEOBJECT)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_SCENEOBJECT))
    {
        rs=App::currentWorld->sceneObjects->getVisionSensorFromHandle(_textureOrVisionSensorObjectID);
        if (rs!=nullptr)
            it=rs->getTextureObject();
    }
    if (it!=nullptr)
    {
        tp->setStartedTextureObject(it);
        _startTextureDisplay(it,tp->getInterpolateColors(),tp->getApplyMode(),tp->getRepeatU(),tp->getRepeatV());
        return(true);
    }
    return(false);
}

void _end3DTextureDisplay(CTextureProperty* tp)
{
    if (tp->getStartedTextureObject()!=nullptr)
        _endTextureDisplay();
    tp->setStartedTextureObject(nullptr);
}

bool _start2DTextureDisplay(CTextureProperty* tp)
{
    int _textureOrVisionSensorObjectID=tp->getTextureObjectID();
    if (_textureOrVisionSensorObjectID==-1)
        return(false);
    CTextureObject* it=nullptr;
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_TEXTURE)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_TEXTURE))
        it=App::currentWorld->textureContainer->getObject(_textureOrVisionSensorObjectID);
    CVisionSensor* rs=nullptr;
    if ((_textureOrVisionSensorObjectID>=SIM_IDSTART_SCENEOBJECT)&&(_textureOrVisionSensorObjectID<=SIM_IDEND_SCENEOBJECT))
    {
        rs=App::currentWorld->sceneObjects->getVisionSensorFromHandle(_textureOrVisionSensorObjectID);
        if (rs!=nullptr)
            it=rs->getTextureObject();
    }
    if (it!=nullptr)
    {
        tp->setStartedTextureObject(it);
        // Following 3 to have "transparency"
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER,0.0f);

        _startTextureDisplay(it,tp->getInterpolateColors(),tp->getApplyMode(),tp->getRepeatU(),tp->getRepeatV());
        return(true);
    }
    return(false);
}

void _end2DTextureDisplay(CTextureProperty* tp)
{
    if (tp->getStartedTextureObject()!=nullptr)
        _endTextureDisplay();
    tp->setStartedTextureObject(nullptr);
    // Following 2 to remove "transparency"
    ogl::setBlending(false);
    glDisable(GL_ALPHA_TEST);
}

void _startTextureDisplay(CTextureObject* to,bool interpolateColor,int applyMode,bool repeatX,bool repeatY)
{ // applyMode: 0=modulate, 1=decal, 2=add (for now)

    int sx,sy;
    to->getTextureSize(sx,sy);
    if (to->getOglTextureName()==(unsigned int)-1)
    {
        unsigned int tn;
        tn=ogl::genTexture();//glGenTextures(1,&tn);
        glBindTexture(GL_TEXTURE_2D,tn);
        to->setOglTextureName(tn);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,to->getTextureBufferPointer());
    }


    glBindTexture(GL_TEXTURE_2D,to->getOglTextureName());
    if (to->getChangedFlag())
    {
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,to->getTextureBufferPointer());
        to->setChangedFlag(false);
    }

    GLint colInt=GL_NEAREST;
    if (interpolateColor)
        colInt=GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // keep to GL_LINEAR here!!
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,colInt);
    GLint repS=GL_CLAMP;
    if (repeatX)
        repS=GL_REPEAT;
    glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,repS);
    GLint repT=GL_CLAMP;
    if (repeatY)
        repT=GL_REPEAT;
    glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,repT);
    GLint dec=GL_MODULATE;
    if (applyMode==1)
        dec=GL_DECAL;
    if (applyMode==2)
        dec=GL_ADD;
    if (applyMode==3)
        dec=GL_BLEND;

    glTexEnvi (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,dec);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,to->getOglTextureName());
    glColor3f(1.0f,1.0f,1.0f);
}

void _endTextureDisplay()
{
    glDisable(GL_TEXTURE_2D);
}

void destroyGlTexture(unsigned int texName)
{
    ogl::delTexture(texName);//glDeleteTextures(1,&texName);
}

#else

void makeColorCurrent(const CColorObject* visParam,bool forceNonTransparent,bool useAuxiliaryComponent)
{

}

void initializeRendering()
{

}

void deinitializeRendering()
{

}

void initGl_ifNeeded()
{ // when calling this we need to have a valid openGl context!!

}

void deinitGl_ifNeeded()
{

}

void increaseVertexBufferRefCnt(int vertexBufferId)
{

}

void decreaseVertexBufferRefCnt(int vertexBufferId)
{

}

void increaseNormalBufferRefCnt(int normalBufferId)
{

}

void decreaseNormalBufferRefCnt(int normalBufferId)
{

}

void increaseEdgeBufferRefCnt(int edgeBufferId)
{

}

void decreaseEdgeBufferRefCnt(int edgeBufferId)
{

}

void decreaseTexCoordBufferRefCnt(int texCoordBufferId)
{

}

void destroyGlTexture(unsigned int texName)
{

}
#endif



