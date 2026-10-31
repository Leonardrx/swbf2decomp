#include "Agent.h"
#include "PebbleFL/Common/PblHashTable.h"

// SYNTHETIC: SWBF2 0x00402190
// `dynamic initializer for 'Agent::rttiHashAgent''

// GLOBAL: SWBF2 0x01eafdfc
// Agent::rttiHashAgent
PblHash Agent::rttiHashAgent("Agent");
const char Agent::rttiNameAgent[] = "Agent";

// FUNCTION: SWBF2 0x004384c0
bool Agent::Update(float dt, bool b)
{
    FSMUpdate(dt, b);
    bool bFinished = (mCurState == 100);
    return bFinished;
}
