
#include "simInternal.h"
#include "outsideCommandQueue.h"
#include "app.h"

const unsigned int MAX_QUEUE_SIZE=64; // no more than 64 messages at a given time!

COutsideCommandQueue::COutsideCommandQueue()
{
}

COutsideCommandQueue::~COutsideCommandQueue()
{ // beware, the current world could be nullptr
}

bool COutsideCommandQueue::addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count)
{ // the queue can't be bigger than 64! (for now)
    // Only for Lua now
    // For the Lua-API:
    if (App::currentWorld->embeddedScriptContainer!=nullptr)
        App::currentWorld->embeddedScriptContainer->addCommandToOutsideCommandQueues(commandID,auxVal1,auxVal2,auxVal3,auxVal4,aux2Vals,aux2Count);
    return(true);
}

void COutsideCommandQueue::newSceneProcedure()
{
//  commands.clear();
//  auxValues.clear();
}
