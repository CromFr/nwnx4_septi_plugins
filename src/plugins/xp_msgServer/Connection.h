#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define DLLEXPORT extern "C" __declspec(dllexport)
// Remembing to define _CRT_RAND_S prior
// to inclusion statement
#define _CRT_RAND_S

#include <memory>
#include <unordered_map>
#include <functional>

#ifdef WIN32
#include <windows.h>
#endif

#include <misc/ini.h>
#include <misc/log.h>
#include <plugins/plugin.h>
#include <NWN2Lib/NWN2.h>
#include <NWN2Lib/NWN2Common.h>

#define SCRIPTRESPONSE_BAN		-3
#define SCRIPTRESPONSE_KICK 	-2
#define SCRIPTRESPONSE_ERROR	-1
#define SCRIPTRESPONSE_NOK		0
#define SCRIPTRESPONSE_OK		1

typedef
bool
(__stdcall*
	MySharedHookFunction) (
		__in int,
		__in unsigned char*,
		__in int);


bool LoadNetLayer();


enum MsgServPlayerStatus
{
	Ban = -2,
	Kick = -1,
	Unknow = 0,
	Know = 1,
	Logged = 2
};

struct PlayerConnection {
	bool m_Know = false;
	std::string m_login;
	bool m_BadPassword = false;
	int m_step;
	MsgServPlayerStatus m_status = MsgServPlayerStatus::Unknow;
};

class MsgServ final : public Plugin
{
  public:
	static constexpr char FunctionClass[] = "MsgServer";

  public:
	  MsgServ();
	~MsgServ();

	bool Init(char* nwnxhome) override;
	int GetInt(char* sFunction, char* sParam1, int nParam2) override;
	void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) override;
	char* GetString(char* sFunction, char* sParam1, int nParam2) override;
	void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) override;
	void GetFunctionClass(char* fClass) override;



	std::string GetVersion() { return version; };
	std::string GetSubClass() { return subClass; };
	
  void RetrieveVersionFromDLL();

  public:

	//LogNWNX* logger;
	SimpleIniConfig* config;
	GameObjectManager m_ObjectManager;
	std::string nwnxStringHome;

	std::unordered_map<std::string, PlayerConnection> _knownPlayers;

	bool bConnectionProcess	= true;
	bool bTraceEveryMsg = false;
	bool bAllowAutoConnect = false;
	bool bWelcomeScreen = false;
	bool bDwnloadPanel = false;
	bool b3CDKey = false;


	//

	bool bAnticheatLvlUp = true;
	bool bACLvlUpStopFirstViolation = true;
	bool bGrantedCondForEveryFeats = false;
	bool bCallScriptOnLvlUpError = false;
	std::string ScriptLvlUpError = "";
	std::string ScriptCreationError = "";
	std::list<int> lRangerCombatFeats;


	std::string ScriptConnectionName = "";
	std::string ScriptStayConnectedScriptName = "";
	std::string ErrorMsg = "";
	std::string sWelcome = "";

	/*
	int currentPlayerPriv_;
	std::string currentName_;
	unsigned long currentIP_;
	std::string currentLog_;
	std::string currentPwd_;
	std::string currentOption_;
	std::string currentCdKey_;
	int currentValidity_ ;
	*/


	std::string curResponseString_;




	
	std::unordered_map<std::string, MySharedHookFunction> _fctList;
	//std::unordered_map<std::string, std::function<bool(int, unsigned char*, int)>> _fctList;

	//unsigned char* DataLogSetTitleText = NULL;
	//unsigned char* DataLogSetLogText = NULL;
	//unsigned char* DataLogSetPwdText = NULL;
	//unsigned long LengthLogTitle = 0;
	//unsigned long LengthLogLog = 0;
	//unsigned long LengthLogPwd = 0;
	unsigned char* DataLogSetRememberMeText = NULL;
	unsigned long LengthLogRememberMe = 0;
	std::string RememberMeText = "";

	unsigned char* DataKickOpen = NULL;
	unsigned long LengthKickPanelOpen = 0;
	std::string KickPanelXml = "";

	unsigned char* DataPopUpOpen = NULL;
	unsigned long LengthPopUpPanelOpen = 0;
	std::string PopUpPanelXml = "";


	std::string LoginXmlFile = "sept_heimdall.xml"



};



