///////////////////////////////////////////////////////////////////////////////////////////
// sept_heimdall_demo_connect - Script used for demo purpose only. Write your own to check
//									and validate the connection of your players.
// Original Scripter:  Septirage
//------------------------------------------------------------------------------
// Last Modified By:   	Septirage			2024/01/31  - Rework with the new version of msgServer
//------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////

#include "nwnx_MsgServer"

/*
 * This function will test if sPlayerName have the right credential and should be allowed to continue or not
 * 
 * sPlayerName      -   The "Bioware" Account of the incoming Player
 * sIp              -   The Ip of incoming player
 * sCDKey           -   The CDKey of the incoming player
 * iPrivileges      -   Current Privileges of the incoming player. Work as itmask with 4 for Admin, 2 for DM, 1 for Player
 *                              IE, to test if someone have all privileges : iPrivileges == 7   /
 *                                  to test if someone have DM : ( (iPrivilege & 2) != 0)
 * sPlayerLogin     -   The login entered on the Connection GUI
 * sPassword        -   The Password entered on the Connection GUI
 * sOption          -   The option parameter from Connection GUI (on default GUI, it's "1" if player ask for "remember me", "0" otherwise)
 *
 * Return value :  HEIMDALL_RET_* constant, depending of the result wanted.
 */
int StartingConditional(string sPlayerName, string sIP, string sCDKey, int iPlayerPrivilege, string sPlayerLogin, string sPassword, string sOption)
{

    // /!\ This is a really dummy implementation, just to show how to work with that script. Don't use it as if.


    //Just for demo purpose: if password = no, refuse connection and prompt "Bad Password"
    if(sPassword == "no")
    {
        XPMsgServer_ScriptResponseMsg("Bad Password");
        return HEIMDALL_RET_NOK;
    }

    //Just for demo purpose : if password = "kick",
    //  kick the player out, with the  message : "Go out bad guy !"
    else if(sPassword == "kick")
    {
        XPMsgServer_ScriptResponseMsg("Go out bad guy !");
        return HEIMDALL_RET_KICK;
    }

    //Just for demo purpose: in any other case, accept the connection (I told you : dummy solution)

        // Player check "Remember me" option so remember their
        if(sOption == "1")
            SetLocalString(GetModule(), "authenticated_" + sPlayerName, sIP);
        else // remove the possible old "remember me" 
            DeleteLocalString(GetModule(), "authenticated_" + sPlayerName);

        
        //In this test, we choose to define the player as "know" only after a successful connection.
        //No more welcome screen for their until reboot.
        XPMsgServer_Known(sPlayerName);

    return HEIMDALL_RET_OK;
}