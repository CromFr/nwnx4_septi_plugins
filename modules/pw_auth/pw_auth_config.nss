#include "nwnx_sql"


//==============================================================================
//                                  BEHAVIOR
//==============================================================================

/// Set to TRUE to allow account registration when a player connects with an
/// unknown account
const int PWAUTH_ALLOW_REGISTRATION = TRUE;

/// Set to TRUE to show a remember me checkbox to future login on the same IP + Account + CDKey
const int PWAUTH_REMEMBER_BUTTON_SHOW = TRUE;

/// Number of successive bad passwords during login before the IP/account is rate-limited.
const int PWAUTH_SECURITY_BADPASSWORD_RETRIES = 4;

/// Duration of the rate-limit when too many bad passwords have been entered
/// See also PWAUTH_MSG_LOGINBLOCKED
const float PWAUTH_SECURITY_BADPASSWORD_COOLDOWN = 1800.0;


//==============================================================================
//                               LOCALIZATION
//==============================================================================

/// Server name
const string PWAUTH_SERVERNAME = "Drizzt PW Server";

/// Error message when a an account already exists but with another case
/// (can cause issues with scripts and database storage)
/// 
/// Tokens:
/// {{CORRECT_ACCOUNT}} Replaced with the correct account name
const string PWAUTH_MSG_KICK_BADCASE = "Your account case is incorrect. Please reconnect to the server using this account: '{{CORRECT_ACCOUNT}}'";

/// Error message when the player account name is unknown and regitration is
/// disabled (PWAUTH_ALLOW_REGISTRATION == FALSE)
const string PWAUTH_MSG_KICK_NEEDREGISTRATION = "You need to register an account before connecting to this server. Go to https://example.com to register yoruself !";

/// Error message when the player has entered the wrong password
const string PWAUTH_MSG_LOGINFAILED = "Your account or password is incorrect";

/// Error message when a player has tried to login with too many wrong
/// passwords and has been rate limited. The rate limit duration is defined
/// by PWAUTH_SECURITY_BADPASSWORD_COOLDOWN
const string PWAUTH_MSG_LOGINBLOCKED = "You have tried too many bad accounts or passwords. Please retry in 30 minutes.";

/// Error message when during registration the player has entered different passwords
const string PWAUTH_MSG_REGISTER_PASSWORDMISMATCH = "The two passwords do not match";

/// Optional message displayed when a player registered their account before
/// selecting their character. Set to "" to disable
const string PWAUTH_MSG_REGISTER_SUCCESS = "Account registered !";


//==============================================================================
//                           DATABASE INTERACTIONS
//==============================================================================

/// Called only once per module boot, to setup the database.
void PWAuth_ModuleInit(){
	SQLExecDirect(
		"CREATE TABLE IF NOT EXISTS `pw_auth_accounts` ("
		+"  `account_name` varchar(32) COLLATE utf8mb4_bin NOT NULL,"
		+"  `password_hash` varchar(512) COLLATE utf8mb4_bin DEFAULT NULL,"
		+"  `password_salt` varchar(32) COLLATE utf8mb4_bin DEFAULT NULL,"
		+"  PRIMARY KEY (`account_name`)"
		+") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"
	);
	SQLExecDirect(
		"CREATE TABLE IF NOT EXISTS `pw_auth_remembered` ("
		+"  `account_name` varchar(32) COLLATE utf8mb4_bin NOT NULL,"
		+"  `ip` varchar(15) COLLATE utf8mb4_bin NOT NULL,"
		+"  `cdkey` varchar(16) COLLATE utf8mb4_bin NOT NULL,"
		+"  PRIMARY KEY (`account_name`,`ip`,`cdkey`),"
		+"  KEY `fk_pw_auth` (`account_name`),"
		+"  CONSTRAINT `fk_pw_auth` FOREIGN KEY (`account_name`) REFERENCES `pw_auth_accounts` (`account_name`) ON DELETE CASCADE ON UPDATE CASCADE"
		+") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;"
	);
}

/// Returns TRUE if the account is already registered and has been remembered
/// on a previous login
int PwAuth_IsRemembered(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	string sPlayerNameEnc = SQLEncodeSpecialChars(sPlayerName);

	SQLExecDirect("SELECT account_name FROM pw_auth_remembered WHERE account_name='" + sPlayerNameEnc + "' AND ip='" + sIP + "' AND cdkey='" + sCDKey + "'");
	return SQLFetch();
}

/// Returns the correct account name for a player if they are registered, or an empty string if the player is not registered.
/// (used for checking if a player is registered or if there is an account case issue)
string PwAuth_GetRegisteredAccount(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	string sPlayerNameEnc = SQLEncodeSpecialChars(sPlayerName);

	// COLLATE utf8mb4_general_ci is used to do a case-insensitive search
	SQLExecDirect("SELECT account_name FROM pw_auth_accounts WHERE account_name='" + sPlayerNameEnc + "' COLLATE utf8mb4_general_ci");
	if(!SQLFetch()){
		// Account is not registered
		return "";
	}
	// Return stored account (for case check)
	return SQLGetData(1);
}


string PwAuth_RegisterNewAccount(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword){
	string sPlayerNameEnc = SQLEncodeSpecialChars(sPlayerName);
	string sPasswordEnc = SQLEncodeSpecialChars(sPassword);

	// Register account with password
	// Player password is never stored in db. The password must be salted then hashed to check that passwords matches without storing the password.
	SQLExecDirect("INSERT INTO pw_auth_accounts (account_name, password_salt, password_hash) VALUES ('" + sPlayerNameEnc + "', MD5(RAND()), SHA2(CONCAT('" + sPasswordEnc + "',password_salt), 512))");

	// Check that the account has been inserted
	if(SQLGetAffectedRows() == 1){
		return "";
	}
	else{
		// This can happen when the SQL query fails
		return "Registration failed. Please contact an admin";
	}
}


int PwAuth_CheckPasswordMatch(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword){
	string sPlayerNameEnc = SQLEncodeSpecialChars(sPlayerName);
	string sPasswordEnc = SQLEncodeSpecialChars(sPassword);

	SQLExecDirect("SELECT SHA2(CONCAT('" + sPasswordEnc + "',`password_salt`), 512)=password_hash as `success` FROM pw_auth_accounts WHERE account_name='" + sPlayerNameEnc + "'");

	return SQLFetch() && StringToInt(SQLGetData(1)) == 1;
}

void PwAuth_RememberPlayer(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	// Remember account+ip+cdkey combination
	string sPlayerNameEnc = SQLEncodeSpecialChars(sPlayerName);
	SQLExecDirect("INSERT INTO pw_auth_remembered (account_name, ip, cdkey) VALUES ('" + sPlayerNameEnc + "', '" + sIP + "', '" + sCDKey + "')");
}




//==============================================================================
//                         ACCOUNT AND PASSWORD POLICY
//==============================================================================

/// Checks if a player is allowed to register the account sPlayerName
///
/// Returns an empty string if the account is acceptable, and an error message otherwise.
string PwAuth_CheckAccountPolicy(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	int bValidAccount = TRUE;

	int nAccountLen = GetStringLength(sPlayerName);

	if(nAccountLen < 3){
		return "Your account name must be longer than 3 characters";
	}
	if(nAccountLen > 32){
		// Adjust max length to fit in the SQL tables
		return "Your account name cannot be longer than 32 characters";
	}
	// if(sCDKey == "01234567"){
	// 	return "This CDKey is not allowed on this server";
	// }
	return "";
}

/// Checks that the player password follows the server rules for secure passwords
///
/// Returns an empty string if the password is valid, and an error message otherwise.
string PwAuth_CheckPasswordPolicy(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sPassword){
	int bValidPassword = TRUE;

	string sPolicy = "Your password must be at least 6 characters, with at least 1 lower case, 1 upper case and 1 number\n\nYour password has the following issues:";

	int nPassLen = GetStringLength(sPassword);
	int nCountLower = 0;
	int nCountUpper = 0;
	int nCountNum = 0;
	int nCountSpec = 0;
	int i;
	for(i = 0 ; i < nPassLen ; i++)
	{
		int nChar = CharToASCII(GetSubString(sPassword, i, 1));
		if(nChar >= 0x30 && nChar <= 0x39)
			nCountNum++;
		else if(nChar >= 0x41 && nChar <= 0x5A)
			nCountUpper++;
		else if(nChar >= 0x61 && nChar <= 0x7A)
			nCountLower++;
		else
			nCountSpec++;
	}

	// At least 6 characters
	if(nPassLen < 6){
		sPolicy += "\nToo short";
		bValidPassword = FALSE;
	}
	if(nCountLower < 1){
		sPolicy += "\nNot enough lower-case characters";
		bValidPassword = FALSE;
	}
	if(nCountUpper < 1){
		sPolicy += "\nNot enough upper-case characters";
		bValidPassword = FALSE;
	}
	if(nCountNum < 1){
		sPolicy += "\nNot enough numbers";
		bValidPassword = FALSE;
	}
	// if(nCountSpec < 1){
	// 	sPolicy += "\nNot enough special characters";
	// 	bValidPassword = FALSE;
	// }

	if(bValidPassword){
		return "";
	}
	return sPolicy;
}





//==============================================================================
//                               GUI CUSTOMIZATION
//==============================================================================

// Custom message box
void PwAuth_OpenMsgGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges, string sMessage){
	XPMsgSrv_Heimdall_DisplayGuiScreen("SCREEN_MESSAGEBOX_DEFAULT", "messageboxdefault.xml");
	XPMsgSrv_Heimdall_SetGUIObjectText("SCREEN_MESSAGEBOX_DEFAULT", "messagetext", sMessage);
}

// Custom login GUI
const string PWAUTH_LOGIN_SCENENAME = "SCREEN_PW_AUTH_LOGIN";
void PwAuth_OpenLoginGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	XPMsgSrv_Heimdall_SetAuthorizedGUIScript("gui_pw_auth_login");
	XPMsgSrv_Heimdall_DisplayGuiScreen(PWAUTH_LOGIN_SCENENAME, "pw_auth_login.xml");

	XPMsgSrv_Heimdall_SetGUIObjectText(PWAUTH_LOGIN_SCENENAME, "SERVERNAME", PWAUTH_SERVERNAME);
	XPMsgSrv_Heimdall_SetGUIObjectText(PWAUTH_LOGIN_SCENENAME, "ACCOUNT", sPlayerName);
	if(!PWAUTH_REMEMBER_BUTTON_SHOW)
		XPMsgSrv_Heimdall_SetGUIObjectHidden(PWAUTH_LOGIN_SCENENAME, "REMEMBER_PANE", TRUE);
}

// Custom registration GUI
const string PWAUTH_REGISTER_SCENENAME = "SCREEN_PW_AUTH_REGISTER";
void PwAuth_OpenRegisterGUI(string sPlayerName, string sIP, string sCDKey, int iPrivileges){
	XPMsgSrv_Heimdall_SetAuthorizedGUIScript("gui_pw_auth_register");
	XPMsgSrv_Heimdall_DisplayGuiScreen(PWAUTH_REGISTER_SCENENAME, "pw_auth_register.xml");

	XPMsgSrv_Heimdall_SetGUIObjectText(PWAUTH_REGISTER_SCENENAME, "SERVERNAME", "Drizzt PW Server");
	XPMsgSrv_Heimdall_SetGUIObjectText(PWAUTH_REGISTER_SCENENAME, "ACCOUNT", sPlayerName);
}