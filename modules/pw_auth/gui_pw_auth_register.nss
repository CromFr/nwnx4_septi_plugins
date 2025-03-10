#include "pw_auth_inc"

int StartingConditional(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sAction, string sPassword, string sPasswordConfirm)
{
	// Stop if the two passwords are different
	if (sPassword != sPasswordConfirm) {
		PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_REGISTER_PASSWORDMISMATCH);
		return XPMSGSRV_HEIMDALL_RET_WAIT;
	}

	string sErrMsg;

	// Stop if the password is not allowed (too short, not random enough, ...)
	sErrMsg = PwAuth_CheckPasswordPolicy(sPlayerName, sIP, sCDKey, iPrivileges, sPassword);
	if (sErrMsg != "") {
		PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, sErrMsg);
		return XPMSGSRV_HEIMDALL_RET_WAIT;
	}

	// Try to register the account
	sErrMsg = PwAuth_RegisterNewAccount(sPlayerName, sIP, sCDKey, iPrivileges, sPassword);
	if (sErrMsg != "") {
		PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, sErrMsg);
		return XPMSGSRV_HEIMDALL_RET_WAIT;
	}

	// Successfully registered account, print success message and close registration UI
	if (PWAUTH_MSG_REGISTER_SUCCESS != "")
		PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_REGISTER_SUCCESS);

	XPMsgSrv_Heimdall_CloseGUIScreen(PWAUTH_REGISTER_SCENENAME);
	return XPMSGSRV_HEIMDALL_RET_ALLOW;
}