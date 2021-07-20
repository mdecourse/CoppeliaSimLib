#include "simplePathPoint_old.h"
#include "tt.h"
#include "simInternal.h"

CSimplePathPoint_old::CSimplePathPoint_old()
{
    commonInit();
}

CSimplePathPoint_old::~CSimplePathPoint_old()
{
}

void CSimplePathPoint_old::commonInit()
{
    _transformation.setIdentity();
    _bezierFactorBefore=0.5f;
    _bezierFactorAfter=0.5f;
    _maxRelAbsVelocity=1.0f;
    _bezierPointCount=15;
    _onSpotDistance=0.0f;
    _auxFlags=0;
    for (int i=0;i<4;i++)
        _auxChannels[i]=0.0f;
}

void CSimplePathPoint_old::setBezierFactors(float fBefore,float fAfter)
{ // 0.99f is so that we don't have dedoubled bezier points!
    tt::limitValue(0.1f,0.99f,fBefore);
    tt::limitValue(0.1f,0.99f,fAfter);
    _bezierFactorBefore=fBefore;
    _bezierFactorAfter=fAfter;
}

void CSimplePathPoint_old::getBezierFactors(float& fBefore,float& fAfter)
{
    fBefore=_bezierFactorBefore;
    fAfter=_bezierFactorAfter;
}

void CSimplePathPoint_old::setBezierPointCount(int c)
{
    c=tt::getLimitedInt(1,100,c);
    if (c<3)
        c=1;
    _bezierPointCount=c;
}

int CSimplePathPoint_old::getBezierPointCount()
{
    return(_bezierPointCount);
}

void CSimplePathPoint_old::scaleYourself(float scalingFactor)
{
    _transformation.X=_transformation.X*scalingFactor;
}

void CSimplePathPoint_old::scaleYourselfNonIsometrically(float x,float y,float z)
{
    _transformation.X(0)=_transformation.X(0)*x;
    _transformation.X(1)=_transformation.X(1)*y;
    _transformation.X(2)=_transformation.X(2)*z;
}


CSimplePathPoint_old* CSimplePathPoint_old::copyYourself()
{ // Everything is copied.
    CSimplePathPoint_old* newPoint=new CSimplePathPoint_old();
    newPoint->_transformation=_transformation;
    newPoint->_bezierFactorBefore=_bezierFactorBefore;
    newPoint->_bezierFactorAfter=_bezierFactorAfter;
    newPoint->_maxRelAbsVelocity=_maxRelAbsVelocity;
    newPoint->_bezierPointCount=_bezierPointCount;
    newPoint->_onSpotDistance=_onSpotDistance;
    newPoint->_auxFlags=_auxFlags;
    for (int i=0;i<4;i++)
        newPoint->_auxChannels[i]=_auxChannels[i];
    return(newPoint);
}

void CSimplePathPoint_old::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("At2");
            ar << _transformation(0) << _transformation(1) << _transformation(2) << _transformation(3);
            ar << _transformation(4) << _transformation(5) << _transformation(6);
            ar << _bezierFactorBefore << _bezierFactorAfter << _bezierPointCount << _maxRelAbsVelocity;
            ar.flush();

            ar.storeDataName("At3");
            ar << _onSpotDistance;
            ar.flush();

            ar.storeDataName("At4");
            ar << _auxFlags;
            ar << _auxChannels[0];
            ar << _auxChannels[1];
            ar << _auxChannels[2];
            ar << _auxChannels[3];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("At2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _transformation(0) >> _transformation(1) >> _transformation(2) >> _transformation(3);
                        ar >> _transformation(4) >> _transformation(5) >> _transformation(6);
                        ar >> _bezierFactorBefore >> _bezierFactorAfter >> _bezierPointCount >> _maxRelAbsVelocity;
                    }
                    if (theName.compare("At3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _onSpotDistance;
                    }
                    if (theName.compare("At4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _auxFlags;
                        ar >> _auxChannels[0];
                        ar >> _auxChannels[1];
                        ar >> _auxChannels[2];
                        ar >> _auxChannels[3];
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("localFrame");
            ar.xmlAddNode_floats("position",_transformation.X.data,3);
            if (exhaustiveXml)
                ar.xmlAddNode_floats("quaternion",_transformation.Q.data,4);
            else
            {
                C3Vector euler(_transformation.Q.getEulerAngles());
                euler*=180.0f/piValue_f;
                ar.xmlAddNode_floats("euler",euler.data,3);
            }
            ar.xmlPopNode();

            ar.xmlPushNewNode("bezierPoints");
            ar.xmlAddNode_int("count",_bezierPointCount);
            ar.xmlAddNode_2float("factors",_bezierFactorBefore,_bezierFactorAfter);
            ar.xmlPopNode();

            ar.xmlAddNode_float("virtualDistance",_onSpotDistance);

            ar.xmlAddNode_int("auxiliaryFlags",_auxFlags);

            ar.xmlAddNode_floats("auxiliaryChannels",_auxChannels,4);
        }
        else
        {
            if (ar.xmlPushChildNode("localFrame",exhaustiveXml))
            {
                ar.xmlGetNode_floats("position",_transformation.X.data,3,exhaustiveXml);
                if (exhaustiveXml)
                {
                    ar.xmlGetNode_floats("quaternion",_transformation.Q.data,4);
                    _transformation.Q.normalize(); // just in case
                }
                else
                {
                    C3Vector euler;
                    if (ar.xmlGetNode_floats("euler",euler.data,3,exhaustiveXml))
                    {
                        euler(0)*=piValue_f/180.0f;
                        euler(1)*=piValue_f/180.0f;
                        euler(2)*=piValue_f/180.0f;
                        _transformation.Q.setEulerAngles(euler);
                    }
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("bezierPoints",exhaustiveXml))
            {
                ar.xmlGetNode_int("count",_bezierPointCount,exhaustiveXml);
                ar.xmlGetNode_2float("factors",_bezierFactorBefore,_bezierFactorAfter,exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_float("virtualDistance",_onSpotDistance,exhaustiveXml);

            int tmp;
            ar.xmlGetNode_int("auxiliaryFlags",tmp,exhaustiveXml);
            _auxFlags=(unsigned short)tmp;

            ar.xmlGetNode_floats("auxiliaryChannels",_auxChannels,4,exhaustiveXml);
        }
    }
}
