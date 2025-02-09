///////////////////////////////////////////////////////////////////////////////////////////
// sept_heimdall_demo_autoconnect - Script used for demo purpose only. Write your own to check 
//										if a person should be automatically reconnected.
// Original Scripter:  Septirage
//------------------------------------------------------------------------------
// Last Modified By:   	Septirage			2024/01/31  - Rework with the new version of msgServer
//------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////

#include "nwnx_MsgServer"

/*
 * This function will test if an incoming player should be automatically connected (HEIMDALL_RET_OK) 
 *      or show the Connection Panel (HEIMDALL_RET_NOK)
 * 
 * sPlayerName      -   The "Bioware" Account of the incoming Player
 * iIp              -   The Ip of incoming player, on int32 format (use XPMsgServer_IpIntToString to convert to string)
 * sCDKey           -   The CDKey of the incoming player
 * iPrivileges      -   Current Privileges of the incoming player. Work as itmask with 4 for Admin, 2 for DM, 1 for Player
 *                              IE, to test if someone have all privileges : iPrivileges == 7   /
 *                                  to test if someone have DM : ( (iPrivileges & 2) != 0)
 *
 * Return value :  HEIMDALL_RET_* constant, depending of the result wanted.
 */
int StartingConditional(string sPlayerName, string sIP, string sCDKey, int iPrivileges)
{
    
    // /!\ This is a really dummy implementation, just to show how to work with that script. Don't use it as if.


    //I don't want to allow "auto-connection" for a "more than player" connection.
    if( (iPrivileges & 6) != 0 ) //We have Admin and or DM right
        return HEIMDAL_RET_NOK;


    //Just between us... I HATE this guy, don't even allow him to continue further
    if( sPlayerName == "I_Love_To_Kill_Gnomes")
        return HEIMDALL_RET_BAN;


    // This player has asked to remain connected and has not changed ip. We connect them without bothering them.
    if(GetLocaString(GetModule(), "authenticated_" + sPlayerName) == sIP)
        return HEIMDALL_RET_OK;

    // Else, continue with the Connection GUI
    return HEIMDALL_RET_NOK;
}
