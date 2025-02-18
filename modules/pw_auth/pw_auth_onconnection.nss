#include "pw_auth_inc"


int StartingConditional(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	WriteTimestampedLogEntry("pw_auth: OnConnection " + sPlayerName + " ip=" + sIP + " cdkey=" + sCDKey + " priv=" + IntToString(iPrivileges));

	if(GetLocalInt(GetModule(), "pw_auth_retries_" + sPlayerName + "@" + sIP) >= PWAUTH_SECURITY_BADPASSWORD_RETRIES){
		// Too many failed login attempts
		PwAuth_OpenKickGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_LOGINBLOCKED);
		return XPMSGSRV_HEIMDALL_RET_KICK;
	}

	if(PwAuth_IsRemembered(sPlayerName, sIP, sCDKey, iPrivileges)){
		// Bypass login if player has logged in with "remember me" ticked
		return XPMSGSRV_HEIMDALL_RET_ALLOW;
	}

	string sKnownAccountName = PwAuth_GetRegisteredAccount(sPlayerName, sIP, sCDKey, iPrivileges);

	if(sKnownAccountName == ""){
		if(PWAUTH_ALLOW_REGISTRATION){
			// Check account policy and kick if unacceptable
			string sErr = PwAuth_CheckAccountPolicy(sPlayerName, sIP, sCDKey, iPrivileges);
			if(sErr != ""){
				PwAuth_OpenKickGUI(sPlayerName, sIP, sCDKey, iPrivileges, sErr);
				return XPMSGSRV_HEIMDALL_RET_KICK;
			}
			
			// Open registration form
			PwAuth_OpenRegisterGUI(sPlayerName, sIP, sCDKey, iPrivileges);
			return XPMSGSRV_HEIMDALL_RET_WAIT;
		}
		else {
			// Kick the player and ask to register elsewhere
			PwAuth_OpenKickGUI(sPlayerName, sIP, sCDKey, iPrivileges, PWAUTH_MSG_KICK_NEEDREGISTRATION);
			return XPMSGSRV_HEIMDALL_RET_KICK;
		}
	}

	if(sKnownAccountName != sPlayerName){
		// Account case issue: kick the player and require to connect with correct account
		string sMsg = ReplaceTokens(PWAUTH_MSG_KICK_BADCASE, "{{CORRECT_ACCOUNT}}", sKnownAccountName);
		PwAuth_OpenKickGUI(sPlayerName, sIP, sCDKey, iPrivileges, sMsg);
		return XPMSGSRV_HEIMDALL_RET_KICK;
	}

	// Open login form
	PwAuth_OpenLoginGUI(sPlayerName, sIP, sCDKey, iPrivileges);
	return XPMSGSRV_HEIMDALL_RET_WAIT;
}