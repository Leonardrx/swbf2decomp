#include "NetGame.h"
#include "Steam/gfGalaxy.h"

namespace FriendBuddyList
{

// FUNCTION: SWBF2 0x0052e140
unsigned int GetFriendBuddyCount()
{
    return gf::friends::friendCount();
}
}

namespace FriendBuddyList
{

// FUNCTION: SWBF2 0x0052e150
int __fastcall GetFriendBuddy(int, unsigned int index)
{
    galaxy::api::GalaxyID id = gf::friends::friendByIndex(index);
    if (id.value)
    {
        return NetGame::FindPlayerByGSID((int)id.value, (int)(id.value >> 32));
    }
    return 0;
}
}

namespace FriendBuddyList
{

// FUNCTION: SWBF2 0x0052e190
bool __fastcall IsFriendBuddy(int, int playerId)
{
    const unsigned int count = FriendBuddyList::GetFriendBuddyCount();
    for (unsigned int i = 0; i < count; ++i)
    {
        if (FriendBuddyList::GetFriendBuddy(0, i) == playerId)
        {
            return true;
        }
    }
    return false;
}
}
