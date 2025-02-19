#include "pw_auth_inc"

void DecrementRetries(string sVarName){
	int nTries = GetLocalInt(GetModule(), sVarName);
	if(nTries > 0){
		SetLocalInt(GetModule(), sVarName, nTries - 1);
	}
}

int StartingConditional(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sAction, string sPassword, string sRemember)
{
	int bRemember = StringToInt(sRemember);
	string sRetryVar = "pw_auth_retries_" + sPlayerName + "@" + sIP;

	if(PwAuth_CheckPasswordMatch(sPlayerName, sIP, sCDKey, iPrivileges, sPassword)){
		// Login success

		// Reset retry count
		DeleteLocalInt(GetModule(), sRetryVar);

		if(bRemember){
			PwAuth_RememberPlayer(sPlayerName, sIP, sCDKey, iPrivileges);
		}

		// Continue to module
		XPMsgSrv_Heimdall_CloseGUIScreen(PWAUTH_LOGIN_SCENENAME);
		return XPMSGSRV_HEIMDALL_RET_ALLOW;
	}


	// Login failed

	// Increment retries count for rate-limiting
	int nTries = GetLocalInt(GetModule(), sRetryVar) + 1;
	SetLocalInt(GetModule(), sRetryVar, nTries);
	DelayCommand(PWAUTH_SECURITY_BADPASSWORD_COOLDOWN, DecrementRetries(sRetryVar));

	// Kick if too many failed attempts
	if(nTries >= PWAUTH_SECURITY_BADPASSWORD_RETRIES){
		PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_LOGINBLOCKED);
		return XPMSGSRV_HEIMDALL_RET_WAIT;
	}

	// Display login failed message
	PwAuth_OpenMsgGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_LOGINFAILED);
	return XPMSGSRV_HEIMDALL_RET_WAIT;
}