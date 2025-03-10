#include "nwnx_msgServer"

void PWAuth_ModuleInit();
int PwAuth_IsRemembered(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
string PwAuth_GetRegisteredAccount(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
string PwAuth_IsLoginAllowed(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
string PwAuth_RegisterNewAccount(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword);
int PwAuth_CheckPasswordMatch(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword);
void PwAuth_RememberPlayer(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
string PwAuth_CheckAccountPolicy(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
string PwAuth_CheckPasswordPolicy(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword);
void PwAuth_OpenMsgGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sMessage);
void PwAuth_OpenLoginGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
void PwAuth_OpenRegisterGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges);
#include "pw_auth_config"

string ReplaceTokens(string sMsg, string sToken, string sValue)
{
	int nTokLen = GetStringLength(sToken);
	int nPos = 0;
	while ((nPos = FindSubString(sMsg, sToken, nPos)) >= 0) {
		sMsg = GetStringLeft(sMsg, nPos) + sValue + GetSubString(sMsg, nPos + nTokLen, -1);
	}
	return sMsg;
}