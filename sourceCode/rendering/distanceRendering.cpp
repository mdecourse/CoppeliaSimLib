#include "distanceRendering.h"
#include "tt.h"
#include "gV.h"

#ifdef SIM_WITH_OPENGL

void displayDistance(CDistanceObject_old* dist,int segWidth,const float* distResult)
{
    dist->getSegmentColor()->makeCurrentColor(false);
    glLineWidth(float(segWidth));
    ogl::drawSingle3dLine(distResult+0,distResult+3,nullptr);
    glLineWidth(1.0f);
    std::string txt("d=");
    txt+=tt::FNb(0,distResult[6],6,false)+" ";
    txt+=gv::getSizeUnitStr();
    ogl::setTextColor(dist->getSegmentColor()->getColorsPtr());
    ogl::drawBitmapTextIntoScene((distResult[0]+distResult[3])/2.0f,(distResult[1]+distResult[4])/2.0f,(distResult[2]+distResult[5])/2.0f,txt.c_str());
}

#else

void displayDistance(CDistanceObject_old* dist,int segWidth,const float* distResult)
{

}

#endif



