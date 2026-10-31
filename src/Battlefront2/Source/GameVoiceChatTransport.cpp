#include "GameVoiceChatTransport.h"
#include "NetGame.h"

extern int netCurMaxPlayers;
namespace FriendBuddyList
{
bool __fastcall IsFriendBuddy(int ignored, int iPlayer);
}

namespace GameVoiceChat
{

// FUNCTION: SWBF2 0x0053dcc0
void Transport::SetSendEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent)
{
    SetSendReceiveEnableUsingMask(node, mask, bEnable, true, bKeepCurrent);
}

// FUNCTION: SWBF2 0x0053dce0
void Transport::SetReceiveEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent)
{
    SetSendReceiveEnableUsingMask(node, mask, bEnable, false, bKeepCurrent);
}

// FUNCTION: SWBF2 0x0053dd00
void Transport::SetSendReceiveEnableUsingMask(const unsigned char nodeID, const Mask mask,
                                              bool enable, const bool send, const bool maskExisting)
{
    int nodeTeam = -1;
    NetPlayerTeamInfo teamInfo[64];
    switch (mask)
    {
    case mask_TeamFriend:
    case mask_TeamEnemy:
    {
        const unsigned int numPlayers = BuildPlayerTeamInfo(teamInfo, 64, nodeID, &nodeTeam);
        for (unsigned int i = 0; i < numPlayers; ++i)
        {
            if (teamInfo[i].player < 0 || teamInfo[i].player == nodeID)
            {
                continue;
            }
            const bool selected = mask == mask_TeamFriend ? teamInfo[i].team == nodeTeam
                                                         : teamInfo[i].team != nodeTeam;
            if (!selected && maskExisting)
            {
                continue;
            }
            const unsigned char remoteNodeID = (unsigned char)teamInfo[i].player;
            if (send)
            {
                SetSendEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetSendEnable(nodeID, remoteNodeID) : enable && selected, false);
            }
            else
            {
                SetReceiveEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetReceiveEnable(nodeID, remoteNodeID) : enable && selected, false);
            }
        }
        break;
    }
    case mask_FriendsBuddies:
    {
        unsigned int iNode = 0;
        const unsigned int numNodes = GetNumNodes();
        for (; iNode < numNodes; ++iNode)
        {
            const unsigned char remoteNodeID = (unsigned char)iNode;
            if (remoteNodeID == nodeID)
            {
                continue;
            }
            const bool friendBuddy = FriendBuddyList::IsFriendBuddy(0, iNode);
            if (!friendBuddy && maskExisting)
            {
                continue;
            }
            if (send)
            {
                SetSendEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetSendEnable(nodeID, remoteNodeID) : enable && friendBuddy, false);
            }
            else
            {
                SetReceiveEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetReceiveEnable(nodeID, remoteNodeID) : enable && friendBuddy, false);
            }
        }
        break;
    }
    case mask_Noone:
        enable = !enable;

    case mask_Everyone:
    {
        unsigned int iNode = 0;
        const unsigned int numNodes = GetNumNodes();
        for (; iNode < numNodes; ++iNode)
        {
            const unsigned char remoteNodeID = (unsigned char)iNode;
            if (remoteNodeID == nodeID)
            {
                continue;
            }
            if (send)
            {
                SetSendEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetSendEnable(nodeID, remoteNodeID) : enable, false);
            }
            else
            {
                SetReceiveEnable(nodeID, remoteNodeID,
                    maskExisting ? enable && GetReceiveEnable(nodeID, remoteNodeID) : enable, false);
            }
        }
        break;
    }
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x0053e040
unsigned int Transport::BuildPlayerTeamInfo(NetPlayerTeamInfo* pInfo, unsigned int uMax, int iNode,
                                            int* pTeam) const
{
    (void)uMax;
    unsigned int uCount = 0;
    for (int iPlayer = 0; iPlayer < netCurMaxPlayers; ++iPlayer)
    {
        if (uCount < 64)
        {
            pInfo[uCount].player = iPlayer;
            const int iTeam = NetGame::GetStartTeam(iPlayer);
            pInfo[uCount].team = iTeam;
            if (pInfo[uCount].player == iNode && pTeam)
            {
                *pTeam = iTeam;
            }
            ++uCount;
        }
    }
    return uCount;
}

}
