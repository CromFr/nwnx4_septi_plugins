////////////////////////////////////////////////////////////////////////////////////////////////
// nwnx_MsgServer - various functions to use the Message server plugin and the connection part
// Original Scripter:  Septirage
//----------------------------------------------------------------------------------------------
// Last Modified By:   Septirage           2024-10-08		Update for v1.1+
// 					   Septirage           2023-01-31
//----------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////


/**************************** General Management ****************************/

//Set the Anticheat LvlUp System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgSrv_SetAntiCheatLvlUpSystem(int bActivated);

//Set the Anticheat Creation System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgSrv_SetAntiCheatCreationSystem(int bActivated);

// //Set the EnforcedSecurity Active or Unactive 
// // bActivated : TRUE / FALSE
// //We recommand to keep it active, unless for debugging  connection issues.
// void XPMsgSrv_SetEnforcedSecurity(int bActivated);

// //Get the state of the EnforcedSecurity
// // TRUE : the enforced security is active. 
// // FALSE otherwise
// int XPMsgSrv_GetEnforcedSecurity();

//Set the whole connection System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgSrv_SetConnectionSystem(int bActivated);


//Unblock the given player
//sPlayerName = "bioware account";
void XPMsgSrv_Unblock(string sPlayerName);

//If you use a WelcomeScreen, use it to set sPlayerName as "known" and so, don't print the screen for they
void XPMsgSrv_Known(string sPlayerName);

/*****************************************************************************
******************* To be used in the ConnectionValidScript ******************
********* Note that these functions can only be used in this script **********
*****************************************************************************/

//Use it to set the resonse text if invalid (for example : "Invalid password")
void XPMsgSrv_ScriptResponseMsg(string sMsg);



/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// IMPLEMENTATIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/



// void XPMsgSrv_SetEnforcedSecurity(int bActivated)
// {
// 	NWNXSetInt("MsgServer", "EnforcedSecurity", "", 0, bActivated);
// }

// int XPMsgSrv_GetEnforcedSecurity()
// {
// 	return NWNXGetInt("MsgServer", "EnforcedSecurity", "", 0);
// }

void XPMsgSrv_SetConnectionSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "H_Enable", "", 0, bActivated);
}

void XPMsgSrv_SetAntiCheatLvlUpSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "AnticheatLvlUp", "", 0, bActivated);
}

void XPMsgSrv_SetAntiCheatCreationSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "AnticheatCreation", "", 0, bActivated);
}

// List of return Constant to use in hookScripts

/// Do nothing and wait for further actions before authenticating the player. Return value used for OnConnection script and related GUI scripts
const int XPMSGSRV_HEIMDALL_RET_WAIT  = 1;
/// Allow player to enter the server. Return value used for OnConnection script and related GUI scripts
const int XPMSGSRV_HEIMDALL_RET_ALLOW   = 2;
/// Kick the player out of the server. Return value used for OnConnection script and related GUI scripts
const int XPMSGSRV_HEIMDALL_RET_KICK = 3;

/// Allows a specific script to be executed by a non authenticated player. This
/// script will have the first 4 arguments provided by MsgSrv, followed by the
/// arguments provided by the UIObject_Misc_ExecuteServerScript GUI callback.
///
/// Only useable in the OnConnection script or related GUI scripts
///
/// The arguments provided by MsgSrv are:
/// - string sAccountName  Player account name (gamespy account, see GetPCPlayerName)
/// - string sCurrentIP    Player IP (see GetPCIPAddress)
/// - string sCDKey        Public part of the player CDKey (see GetPCPublicCDKey)
/// - int nPrivileges      Player/DM/Admin privileges (see XPMsgSrv_Heimdall_GetIsXXX)
void XPMsgSrv_Heimdall_SetAuthorizedGUIScript(string sScript)
{
	NWNXSetString("MsgServer", "H_SetGUIScript", "", 0, sScript);
}


/// Opens the given GUI for the currenlty connecting player. See DisplayGuiScreen
///
/// Only useable in the OnConnection script or related GUI scripts
void XPMsgSrv_Heimdall_DisplayGuiScreen(string sScreenName, string sFileName)
{
	NWNXSetString("MsgServer", "H_DisplayGuiScreen", sScreenName, 0, sFileName);
}

/// Close the given GUI for the currenlty connecting player. See CloseGUIScreen
///
/// Only useable in the OnConnection script or related GUI scripts
void XPMsgSrv_Heimdall_CloseGUIScreen(string sScreenName)
{
	NWNXSetString("MsgServer", "H_CloseGUIScreen", sScreenName, 0, "");
}

/// Show or hide a GUI object for the currenlty connecting player. See SetGUIObjectHidden
///
/// Only useable in the OnConnection script or related GUI scripts
void XPMsgSrv_Heimdall_SetGUIObjectHidden(string sScreenName, string sUIObjectName, int bHidden)
{
	NWNXSetString("MsgServer", "H_SetGUIObjectHidden", sScreenName + ">" + sUIObjectName, bHidden, "");
}

/// Set the text of a GUI UIText for the currenlty connecting player. See SetGUIObjectText
///
/// Only useable in the OnConnection script or related GUI scripts
void XPMsgSrv_Heimdall_SetGUIObjectText(string sScreenName, string sUIObjectName, string sText)
{
	NWNXSetString("MsgServer", "H_SetGUIObjectText", sScreenName + ">" + sUIObjectName, 0, sText);
}

/// Returns TRUE if iPrivileges corresponds to a player connecting with the game client
int XPMsgSrv_Heimdall_GetIsPlayer(int iPrivileges){
	return (iPrivileges & 1) != 0;
}
/// Returns TRUE if iPrivileges corresponds to a DM connecting with the DM client
int XPMsgSrv_Heimdall_GetIsDM(int iPrivileges){
	return (iPrivileges & 2) != 0;
}
/// Returns TRUE if iPrivileges corresponds to an admin connecting with the server admin interface
int XPMsgSrv_Heimdall_GetIsAdmin(int iPrivileges){
	return (iPrivileges & 4) != 0;
}