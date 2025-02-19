#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define DLLEXPORT extern "C" __declspec(dllexport)
// Remembing to define _CRT_RAND_S prior
// to inclusion statement
#define _CRT_RAND_S

#include <memory>
#include <unordered_map>
#include <functional>
#include <variant>
#include <optional>
#include <unordered_set>

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

	std::unique_ptr<SimpleIniConfig> config;
	GameObjectManager m_ObjectManager;
	std::string nwnxStringHome;

	bool bTraceEveryMsg = false;
	bool b3CDKey = false;


	//

	bool bAnticheatLvlUp = true;
	bool bACLvlUpStopFirstViolation = true;
	bool bGrantedCondForEveryFeats = false;
	bool bCallScriptOnLvlUpError = false;
	std::string ScriptLvlUpError = "";
	std::string ScriptCreationError = "";
	std::list<int> lRangerCombatFeats;


	struct {
		enum ScriptRet{
			WAIT  = 1,
			ALLOW = 2,
			KICK  = 3,
		};

		bool enabled;
		bool strict;

		std::string onConnectionScript;

		// This variable is only set during the execution of OnConnected script
		std::optional<std::tuple<unsigned long, std::string>> currentPlayerInfo;

		std::unordered_map<unsigned long, std::string> authorizedGUIScripts;

		std::unordered_set<unsigned long> authorizedPlayerIDs;
	} m_heimdall;

	void NWNX_SetAuthorizedGUIScript(const std::string_view& scriptName);
	void NWNX_DisplayGUIScreen(const std::string_view& sceneName, const std::string_view& xmlName);
	void NWNX_CloseGUIScreen(const std::string_view& sceneName);
	void NWNX_SetGUIObjectHidden(const std::string_view& sceneName, const std::string_view& objectName, bool hidden);
	void NWNX_SetGUIObjectText(const std::string_view& sceneName, const std::string_view& objectName, std::string_view text);
};



