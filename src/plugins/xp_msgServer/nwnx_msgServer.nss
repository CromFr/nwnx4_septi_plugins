////////////////////////////////////////////////////////////////////////////////////////////////
// nwnx_MsgServer - various functions to use the Message server plugin and the connection part
// Original Scripter:  Septirage
//----------------------------------------------------------------------------------------------
// Last Modified By:   Septirage           2024-10-08		Update for v1.1+
// 					   Septirage           2023-01-31
//----------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////

// List of return Constant to use in hookScripts
// Note : Ban will "only" block all communication from the 
//			current PlayerName until next serverReboot or call of XPMsgServer_Unblock
const int HEIMDALL_RET_BAN		=  -3;
const int HEIMDALL_RET_KICK		=  -2;
const int HEIMDALL_RET_NOK		=	0;
const int HEIMDALL_RET_OK		=	1;

/**************************** General Management ****************************/

//Set the Anticheat LvlUp System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgServer_SetAntiCheatLvlUpSystem(int bActivated);

//Set the Anticheat Creation System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgServer_SetAntiCheatCreationSystem(int bActivated);

//Set the EnforcedSecurity Active or Unactive 
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgServer_SetEnforcedSecurity(int bActivated);

//Get the state of the EnforcedSecurity
// TRUE : the enforced security is active. 
// FALSE otherwise
int XPMsgServer_GetEnforcedSecurity();

//Set the whole connection System Active or Unactive
// bActivated : TRUE / FALSE
//We recommand to keep it active, unless for debugging  connection issues.
void XPMsgServer_SetConnectionSystem(int bActivated);


//Unblock the given player
//sPlayerName = "bioware account";
void XPMsgServer_Unblock(string sPlayerName);

//If you use a WelcomeScreen, use it to set sPlayerName as "known" and so, don't print the screen for they
void XPMsgServer_Known(string sPlayerName);

/*****************************************************************************
******************* To be used in the ConnectionValidScript ******************
********* Note that these functions can only be used in this script **********
*****************************************************************************/

//Use it to set the resonse text if invalid (for example : "Invalid password")
void XPMsgServer_ScriptResponseMsg(string sMsg);



/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// IMPLEMENTATIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/

void XPMsgServer_SetEnforcedSecurity(int bActivated)
{
	NWNXSetInt("MsgServer", "EnforcedSecurity", "", 0, bActivated);
}

int XPMsgServer_GetEnforcedSecurity()
{
	return NWNXGetInt("MsgServer", "EnforcedSecurity", "", 0);
}

void XPMsgServer_ScriptResponseMsg(string sMsg)
{
	NWNXSetString("MsgServer", "Response", "", 0, sMsg);
}

void XPMsgServer_Unblock(string sPlayerName)
{
	NWNXSetInt("MsgServer", "Unblock", sPlayerName, 0, 0);
}

void XPMsgServer_Known(string sPlayerName)
{
	NWNXSetInt("MsgServer", "Known", sPlayerName, 0, 1);
}

void XPMsgServer_SetConnectionSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "ConnectionSystem", "", 0, bActivated);
}

void XPMsgServer_SetAntiCheatLvlUpSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "AnticheatLvlUp", "", 0, bActivated);
}

void XPMsgServer_SetAntiCheatCreationSystem(int bActivated)
{
	NWNXSetInt("MsgServer", "AnticheatCreation", "", 0, bActivated);
}

int XPMsgServer_GetIsPlayer(int iPrivileges){
	return (iPrivileges & 1) != 0;
}
int XPMsgServer_GetIsDM(int iPrivileges){
	return (iPrivileges & 2) != 0;
}
int XPMsgServer_GetIsAdmin(int iPrivileges){
	return (iPrivileges & 4) != 0;
}

void XPMsgServer_SetLoginXml(string sLoginScreenXml)
{
	NWNXSetString("MsgServer", "SetLoginXml", "", 0, sLoginScreenXml);
}