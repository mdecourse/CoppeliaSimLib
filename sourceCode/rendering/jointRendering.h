#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _displayJoint(CJoint* joint,int displayAttrib,bool partOne,float sizeParam);
#endif

void displayJoint(CJoint* joint,CViewableBase* renderingObject,int displayAttrib);
