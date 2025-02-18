#include "nwnx_msgServer"

void PwAuth_OpenMsgGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sMessage);
void PwAuth_OpenLoginGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
void PwAuth_OpenRegisterGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
void PwAuth_OpenKickGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sMessage);

// TODO: declare everything

#include "pw_auth_config"


string ReplaceTokens(string sMsg, string sToken, string sValue){
	int nTokLen = GetStringLength(sToken);
	int nPos = 0;
	while((nPos = FindSubString(sMsg, sToken, nPos)) >= 0){
		sMsg = GetStringLeft(sMsg, nPos) + sValue + GetSubString(sMsg, nPos + nTokLen, -1);
	}
	return sMsg;
}