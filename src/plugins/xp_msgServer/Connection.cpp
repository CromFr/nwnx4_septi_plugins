#include "Connection.h"
#include "messageConst.h"

#include <NWN2Lib/NWN2.h>
#include <NWN2Lib/NWN2Common.h>
#include <cstring>
#include <hook/scriptManagement.h>
#include <misc/log.h>
#include <nwn2heap.h>
#include "Anticheat.h"

#include <bit>
#include <cassert>
#include <charconv>
#include <detours/detours.h>
#include <optional>
#include <string_view>
#include <iostream>
#include <array>
#include <span>
#include <fstream>
#include "../../septutil/srvadmin.h"
#include "../../septutil/win_utils.h"
#include "../../septutil/bytearray.h"

#define MAX_PLAYERS               0x60

#define G_GLOBAL_GAME_STATE		0x86443C

HMODULE XPBugfix;

std::unique_ptr<LogNWNX> logger;

namespace {
	auto plugin = std::unique_ptr<MsgServ>();
}

typedef
BOOL
(__stdcall *
	SendMessageToPlayer)(
		__in unsigned long PlayerId,
		__in_bcount(Size) unsigned char* Data,
		__in unsigned long Size,
		__in unsigned long Flags
		);

SendMessageToPlayer     SendMessageToPlayer_;

typedef
void
(__stdcall * OnPlayerConnectionCloseProc)(
	__in unsigned long PlayerId,
	__in void * Context
	);

typedef
BOOL
(__stdcall * OnPlayerConnectionReceiveProc)(
	__in unsigned long PlayerId,
	__in_bcount( Length ) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
	);

typedef
BOOL
(__stdcall * OnPlayerConnectionSendProc)(
	__in unsigned long PlayerId,
	__in_bcount( Size ) unsigned char * Data,
	__in unsigned long Size,
	__in unsigned long Flags,
	__in void * Context
	);


typedef
void
(__stdcall *
	SetPacketFilterCallouts)(
		__in void* Context,
		__in OnPlayerConnectionCloseProc OnClose,
		__in OnPlayerConnectionReceiveProc OnReceive,
		__in OnPlayerConnectionSendProc OnSend
		);

SetPacketFilterCallouts     SetPacketFilterCallouts_;


struct CPlayerCDKeyInfo
{
	NWN::CExoString m_Key;
	NWN::CExoString m_ValidCode;
	NWN::CExoString m_NotUsed;
};

struct BigPlayerInfo // sizeof = 0x78, CNetLayerPlayerInfo
{
	int            m_bPlayerInUse;              // 00
	NWN::CExoString     m_sPlayerName;               // 04
	char           skip0[0x04];                 // 0c
	unsigned long  m_nSlidingWindowId;          // 10
	int            m_bPlayerPrivileges;         // 14
	int            m_bGameMasterPrivileges;     // 18
	int            m_bServerAdminPrivileges;    // 1c
	char           skip1[0x38];                 // 20
	CPlayerCDKeyInfo* m_lstKeys;				// 58
	int				m_nNumberKeys;				// 5C
	char			skip2[0x18];			// 60
};


struct CShortNetLayerInternal
{
	void         *ServerApp;                 // 00000
	char		 skip0[0x3768C];
	//CExoNet      *Net;                       // 00004
	//char          skip0[0x04];               // 00008
	//SlidingWindow Windows[MAX_PLAYERS];      // 0000c
	//char          skip1[0x04];               // 3768C
	BigPlayerInfo    Players[MAX_PLAYERS];      // 37690
											 // CExoNetExtendableBuffer FrameStorage; // 3A390
};


typedef
const char *
(__stdcall *
GetPlayerAccountName)(
	__in unsigned long PlayerId
);


GetPlayerAccountName     GetPlayerAccountName_;


typedef
bool
(__stdcall*
	GetPlayerConnectionInfo)(
		__in unsigned long PlayerId,
		__out PSOCKADDR_IN Sin
		);

GetPlayerConnectionInfo GetPlayerConnectionInfo_;


	MsgServ* g_msgServ;

//OFFS_g_pAppManager
//Must be redone to be based on struct instead ugly ptr management
uint8_t* GetPlayerStruct(uint8_t idPlayer)
{
	int var = *(int*)OFFS_g_pAppManager;
	var = var + 4;
	var = *(int*)var;
	int var2 = *(int*)var;
	var2 += 0x1C;
	var2 = *(int*)var2;
	
	//VAR
	var += 4;
	var = *(int*)var;
	var += 0x10068;
	var = *(int*)var;
	var = *(int*)var;



	var += 0x37690;

	var += (idPlayer * 0x78);

	return (uint8_t*)var;
}

//Must be redone to be based on struct instead ugly ptr management
std::string GetCDKey(uint8_t idPlayer)
{
	uint8_t* pMyPlayerStruct = GetPlayerStruct(idPlayer);
	if (pMyPlayerStruct == NULL)
		return "";
	uint8_t* pMyCDKey = (uint8_t*)(((int)pMyPlayerStruct) + 0x58);

	if (pMyCDKey == NULL)
		return "";
	pMyCDKey = *(uint8_t**)(pMyCDKey);


	if (pMyCDKey == NULL)
		return "";

	char* pCDKey1 = *(char**)(pMyCDKey);
	std::string myCdKey = "";
	for (int i = 0; i < 8; i++)
	{
		myCdKey += pCDKey1[i];
	}

	return myCdKey;
}


//Just create a privileges flag based uint8
int GetPlayerPrivileges(int playerid)
{
	uint8_t* pMyPlayerStruct = GetPlayerStruct(playerid);
	int m_bPlayerPrivileges = *(int*)(pMyPlayerStruct + 0x14);
	int m_bGameMasterPrivileges = *(int*)(pMyPlayerStruct + 0x18);
	int m_bServerAdminPrivileges = *(int*)(pMyPlayerStruct + 0x1C);

	int result = 0;
	if (m_bPlayerPrivileges != 0)
		result |= 1;
	if (m_bGameMasterPrivileges != 0)
		result |= 2;
	if (m_bServerAdminPrivileges != 0)
		result |= 4;
	return result;
}




// std::string formatMessage(const std::span<const uint8_t>& message)
// {
// 	std::string sResult = "";
// 	char buffer[3];
// 	for (size_t i = 0; i < message.size(); ++i) {
// 		sprintf(buffer, "%02X ", message[i]);
// 		sResult += buffer;
// 	}
// 	return sResult;
// }


void SendOpenGUI(unsigned long playerID, const std::string_view& sceneName, const std::string_view& xmlFileName){
	uint32_t msgDataSize = 3 + 4 + 4 + sceneName.size() + 4 + xmlFileName.size();
	auto msgData = ByteArray{msgDataSize + 1};

	msgData.extend({0x50, 0x24, 0x04});
	msgData.extend((uint32_t)msgDataSize);
	msgData.extend((uint32_t)sceneName.size());
	msgData.extend(sceneName);
	msgData.extend((uint32_t)xmlFileName.size());
	msgData.extend(xmlFileName);
	msgData.extend({0xB3});

	// const auto msgStr = formatMessage(msgData.data);
	// logger->Trace("%s: %s", __FUNCTION__, msgStr.c_str());
	SendMessageToPlayer_(playerID, msgData.data.data(), msgData.data.size(), 0);
}
void SendCloseGUI(unsigned long playerID, const std::string_view& sceneName){
	uint32_t msgDataSize = 3 + 4 + 4 + sceneName.size();
	auto msgData = ByteArray{msgDataSize + 1};

	msgData.extend({0x50, 0x24, 0x07});
	msgData.extend((uint32_t)msgDataSize);
	msgData.extend((uint32_t)sceneName.size());
	msgData.extend(sceneName);
	msgData.extend({0x73});

	// const auto msgStr = formatMessage(msgData.data);
	// logger->Trace("%s: %s", __FUNCTION__, msgStr.c_str());
	SendMessageToPlayer_(playerID, msgData.data.data(), msgData.data.size(), 0);
}

void SendSetGUIObjectHidden(unsigned long playerID, const std::string_view& sceneName, const std::string_view& sUIObjectName, bool hidden){
	uint32_t msgDataSize = 3 + 4 + 4 + sceneName.size() + 4 + sUIObjectName.size();
	auto msgData = ByteArray{msgDataSize + 1};

	msgData.extend({0x50, 0x24, 0x06});
	msgData.extend((uint32_t)msgDataSize);
	msgData.extend((uint32_t)sceneName.size());
	msgData.extend(sceneName);
	msgData.extend((uint32_t)sUIObjectName.size());
	msgData.extend(sUIObjectName);
	if(hidden)
		msgData.extend({0x97});
	else
		msgData.extend({0x87});


	// const auto msgStr = formatMessage(msgData.data);
	// logger->Trace("%s: %s", __FUNCTION__, msgStr.c_str());
	SendMessageToPlayer_(playerID, msgData.data.data(), msgData.data.size(), 0);
}

void SendSetGuiObjectText(unsigned long playerID, const std::string_view& sceneName, const std::string_view& sUIObjectName, const std::string_view& sText){
	uint32_t msgDataSize = 3 + 4 + 4 + sceneName.size() + 4 + sUIObjectName.size() + 4 + sText.size();
	auto msgData = ByteArray{msgDataSize + 1};

	msgData.extend({0x50, 0x24, 0x0A});
	msgData.extend((uint32_t)msgDataSize);
	msgData.extend((uint32_t)sceneName.size());
	msgData.extend(sceneName);
	msgData.extend((uint32_t)sUIObjectName.size());
	msgData.extend(sUIObjectName);
	msgData.extend((uint32_t)sText.size());
	msgData.extend(sText);
	msgData.extend({0x93});

	// const auto msgStr = formatMessage(msgData.data);
	// logger->Trace("%s: %s", __FUNCTION__, msgStr.c_str());
	SendMessageToPlayer_(playerID, msgData.data.data(), msgData.data.size(), 0);
}



void PrintFullMessage(const unsigned char* Data, size_t size, std::string sCharName)
{
	if(logger->Level() >= LogLevel::info)
	{
		std::string sResult = "";
		char buffer[3];
		for (size_t i = 0; i < size; ++i) {
			sprintf(buffer, "%02X ", Data[i]);
			sResult += buffer;
		}
		logger->Info("Message from %s: \n\t %s", sCharName.c_str(), sResult.c_str());
	}
}

template<typename T>
bool operator==(const std::span<const T> a, const std::initializer_list<T>& b){
	if(a.size() != b.size())
		return FALSE;
	else
		return memcmp(a.data(), std::data(b), a.size()) == 0;
}

BOOL __stdcall MsgServOnReceive(
	__in unsigned long playerId,
	__in_bcount(Length) const unsigned char * Data,
	__in size_t Length,
	__in void * Context
) {
	auto data = std::span{(const uint8_t*)Data, Length};
	try
	{
		logger->Err("Recv msg from playerid %lu", playerId);
		const char* accountName = GetPlayerAccountName_(playerId);

		//Only if we want to trace everything
		if (g_msgServ->bTraceEveryMsg)
		{
			PrintFullMessage(Data, Length, accountName);
		}

		//Only if we want to active loggin module.
		if (g_msgServ->m_heimdall.enabled)
		{

			//Will not manage bad message
			if(data.size() > 1 && data[0] == 0x70){

				//Module.ModuleLoaded. "Initial message"
				if(data.size() == 10 && data.subspan(1, 8) == std::initializer_list<uint8_t>{0x03, 0x02, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00}){
					// OnConnection
					if(!g_msgServ->m_heimdall.onConnectionScript.empty())
					{
						// Reset player authorizations
						g_msgServ->m_heimdall.authorizedPlayerIDs.erase(playerId);


						std::string currentIP_ = [&](){
							sockaddr_in sin;
							GetPlayerConnectionInfo_(playerId, &sin);
							const auto ip = sin.sin_addr.s_addr;
							return std::format("{}.{}.{}.{}", (ip>>24) & 0xFF, (ip>>16) & 0xFF, (ip>>8) & 0xFF, ip & 0xFF);
						}();
						std::string currentCdKey_ = GetCDKey(playerId);
						int currentPlayerPriv_ = GetPlayerPrivileges(playerId);
						bool isExecScriptOk = false;

						g_msgServ->m_heimdall.currentPlayerInfo = {playerId, accountName};
						g_msgServ->m_heimdall.authorizedGUIScripts.erase(playerId);

						NWScript::ClearScriptParams();
						NWScript::AddScriptParameterString(accountName);
						NWScript::AddScriptParameterString(currentIP_.c_str());
						NWScript::AddScriptParameterString(currentCdKey_.c_str());
						NWScript::AddScriptParameterInt(currentPlayerPriv_);
						int scriptRes = NWScript::ExecuteScriptEnhanced(g_msgServ->m_heimdall.onConnectionScript.c_str(), 0, true, &isExecScriptOk, true);

						g_msgServ->m_heimdall.currentPlayerInfo.reset();

						if(!isExecScriptOk)
						{
							//Error on script, log and continue as not autoconnected.
							logger->Err("Failed to execute Heimdall OnConnection Script (%s) params : %s, %s, %s, %d",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_
							);
						}
						else{
							logger->Trace("Executed Heimdall OnConnection Script (%s) params : %s, %s, %s, %d, returned %d",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_,
								scriptRes
							);
						}

						using ScriptRet = decltype(MsgServ::m_heimdall)::ScriptRet;
						switch(scriptRes){
						case ScriptRet::ALLOW:
							g_msgServ->m_heimdall.authorizedPlayerIDs.insert(playerId);
							break;
						case ScriptRet::WAIT:
							break;
						case ScriptRet::KICK:
							return false;
						default:
							//Error on script, log and continue as not autoconnected.
							logger->Err("Heimdall OnConnection Script (%s) returned error value %d. params : %s, %s, %s, %d",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								scriptRes,
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_
							);
							return false;
						}
					}
				}
				// GUI script execution request
				else if (data.size() >= 11 && data.subspan(1, 2) == std::initializer_list<uint8_t>{0x06, 0x30}){
					const auto scriptNameLen = *(uint32_t*)&data[7];
					const auto scriptName = std::string{(const char*)&data[11], scriptNameLen};

					if(const auto match = g_msgServ->m_heimdall.authorizedGUIScripts.find(playerId)
					   ; match != g_msgServ->m_heimdall.authorizedGUIScripts.cend() && match->second == scriptName){

					   	size_t offset = 11 + scriptNameLen;
						if(data.size() < offset)
							return FALSE;
					   	uint8_t argsCount = data[offset];
					   	offset += 1;

					   	std::vector<std::string> extraStringArgs;
						for(uint8_t i = 0 ; i < argsCount ; i++){
							if(offset + 4 > data.size())
								break;
							const auto length = *(uint32_t*)&data[offset];
							offset += 4;
							if(offset + length > data.size())
								break;
							const auto value = std::string{(const char*)&data[offset], length};
							offset += length;

							extraStringArgs.push_back(value);
						}

						std::string currentIP_ = [&](){
							sockaddr_in sin;
							GetPlayerConnectionInfo_(playerId, &sin);
							const auto ip = sin.sin_addr.s_addr;
							return std::format("{}.{}.{}.{}", (ip>>24) & 0xFF, (ip>>16) & 0xFF, (ip>>8) & 0xFF, ip & 0xFF);
						}();
						std::string currentCdKey_ = GetCDKey(playerId);
						int currentPlayerPriv_ = GetPlayerPrivileges(playerId);
						bool isExecScriptOk = false;

						g_msgServ->m_heimdall.currentPlayerInfo = {playerId, accountName};

						NWScript::ClearScriptParams();
						NWScript::AddScriptParameterString(accountName);
						NWScript::AddScriptParameterString(currentIP_.c_str());
						NWScript::AddScriptParameterString(currentCdKey_.c_str());
						NWScript::AddScriptParameterInt(currentPlayerPriv_);

						// Prevent logger from logging password
						const auto logLevel = logger->Level();
						if(logLevel >= LogLevel::trace){
							logger->SetLogLevel(LogLevel::debug);
						}
						for(const auto& arg : extraStringArgs){
							NWScript::AddScriptParameterString(arg.c_str());
						}
						if(logLevel >= LogLevel::trace){
							logger->SetLogLevel(logLevel);
						}
						int scriptRes = NWScript::ExecuteScriptEnhanced(scriptName.c_str(), 0, true, &isExecScriptOk, true);

						g_msgServ->m_heimdall.currentPlayerInfo.reset();

						if(!isExecScriptOk)
						{
							//Error on script, log and continue as not autoconnected.
							logger->Err("Failed to execute Heimdall GUI Script (%s) params : %s, %s, %s, %d, +%d GUI args",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_,
								extraStringArgs.size()
							);
						}
						else{
							logger->Trace("Executed Heimdall GUI Script (%s) params : %s, %s, %s, %d, +%d GUI args, returned %d",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_,
								extraStringArgs.size(),
								scriptRes
							);
						}


						using ScriptRet = decltype(MsgServ::m_heimdall)::ScriptRet;
						switch(scriptRes){
						case ScriptRet::ALLOW:
							g_msgServ->m_heimdall.authorizedPlayerIDs.insert(playerId);
							break;
						case ScriptRet::WAIT:
							break;
						case ScriptRet::KICK:
							return false;
						default:
							//Error on script, log and continue as not autoconnected.
							logger->Err("Heimdall GUI Script (%s) returned error value %d. params : %s, %s, %s, %d, [redacted]",
								g_msgServ->m_heimdall.onConnectionScript.c_str(),
								scriptRes,
								accountName,
								currentIP_.c_str(),
								currentCdKey_.c_str(),
								currentPlayerPriv_
							);
							return false;
						}

					}
				}
				else{
					if(!g_msgServ->m_heimdall.authorizedPlayerIDs.contains(playerId)){
						//CharList, never allow that without login
						if (data[1] == 0x11)
						{
							return false;
						}
						else if (data[1] == 0x2)
						{
							unsigned char cSubType = data[2];
							//Don't allow to create or load character
							if (cSubType == 0x01 || cSubType == 0x02 || cSubType == 0x04 ||
								cSubType == 0x0e || cSubType == 0x0f || cSubType == 0x11 ||
								cSubType == 0x13)
							{
								return false;	
							}
						}
					}
				}
			}

		}

		//Only if we want to active Anticheat module.
		{
			//EndOfLevelUp
			if (Data[0] == 0x70 && Data[1] == 0x1D && Data[2] == 0x00 && g_msgServ->bAnticheatLvlUp)
			{
				int iResLvlUp = CheckForLevelUp(playerId, Data, Length, accountName, g_msgServ);
				if (iResLvlUp <= 0)
					return FALSE;
			}
		}
	}
	catch (std::exception& e)
	{
		std::string nomExcp(e.what());
		std::string logTxt2 = "Exception : " + nomExcp;
		logger->Err(logTxt2.c_str());
	}
	catch (...)
	{
		std::string logTxt2 = "Exception ! => " + std::to_string(Length) + ", id : " + std::to_string(playerId) + ")";
		logger->Err(logTxt2.c_str());
	}

	return TRUE;
}

DLLEXPORT Plugin*
GetPluginPointerV2()
{
	return plugin.get();
}

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		plugin = std::make_unique<MsgServ>();

		char szPath[MAX_PATH];
		GetModuleFileNameA(hModule, szPath, MAX_PATH);
		plugin->SetPluginFullPath(szPath);
		plugin->RetrieveVersionFromDLL();
	} else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		plugin.reset();
	}
	return TRUE;
}

MsgServ::MsgServ()
{
	description = "This plugin provides script access to server received msg.";

	subClass = FunctionClass;
	version  = "unknown";
}

MsgServ::~MsgServ(void) {}

void MsgServ::RetrieveVersionFromDLL(){
	if(auto ver = GetDLLVersion(GetPluginFullPath())){
		version = ver.value();
	}
}

DWORD WINAPI LaunchTestVersion(LPVOID lpParam)
{
	TstVersionPlg testPlugin(g_msgServ->nwnxStringHome, g_msgServ->GetVersion(), g_msgServ->GetSubClass());
	testPlugin.TestVersionPlugin();
	return 0;
}

bool
MsgServ::Init(char* nwnxhome)
{
	nwnxStringHome = nwnxhome;
		/* Log file */
	std::string logfile(nwnxhome);
	logfile.append("\\");
	logfile.append(GetPluginFileName());
	logfile.append(".txt");

	/* Ini file */
	
	std::string inifile(nwnxhome);
	inifile.append("\\");
	inifile.append(GetPluginFileName());
	inifile.append(".ini");

	std::string header = "NWNX MsgServ Plugin v"+version+"\n"
	                     "(c) 2024 by Septirage\n"
	                     "visit us at http://septirage.com/nwn2/ \n"
	                     "visit nwnx project at http://www.nwnx.org\n";

	logger = std::make_unique<LogNWNX>(logfile);
	logger->Info(header.c_str());

	 logger->Trace("* reading inifile %s", inifile.c_str());

	 config = std::make_unique<SimpleIniConfig>(inifile);

	 logger->Configure(config.get());

	 // Heimdall
	 config->Read<bool>("EnableConnectionSystem", &m_heimdall.enabled, true);
	 if (m_heimdall.enabled) {
		 logger->Debug("EnableConnectionSystem set to true.");
	 }
	 else {
		 logger->Debug("EnableConnectionSystem set to false.");
	 }

	 config->Read<std::string>("OnConnectionScript", &m_heimdall.onConnectionScript, "");
	 if (!m_heimdall.onConnectionScript.empty()) {
		 logger->Debug("Authentication active with script %s", m_heimdall.onConnectionScript.c_str());
	 }
	 else {
		 logger->Debug("Authentication un-available (no script passed)");
	 }


	 /*
	#
	# Mostly for debug purpose. If you are at least in loglevel info, it will 
	# trace every messages received by the server.
	# Should probably not be used in prod
	#
	# LogEveryMsg = 0
	*/
	 config->Read<bool>("LogEveryMsg", &bTraceEveryMsg, false);
	 if (bTraceEveryMsg)
	 {
		 logger->Debug("Will log every message received");
	 }

	 config->Read<bool>("MCDKey", &b3CDKey, false);




	 //Anticheat system
	 {
		 int iAnticheatCreation = 0;

		 config->Read("UseAnticheatCreation", &iAnticheatCreation, 1);
		// if (iAnticheatCreation != 0)
		 {
			 ApplyAntiCheatCreationPatch(*config, iAnticheatCreation != 0);
		 }


		 int iAnticheatLvlUp = 0;
		 int iACLvlUpStopFirstViolation = 0;
		 int iGrantedCondForEveryFeats = 0;
		 int iCallScriptOnLvlUpError = 0;

		 config->Read("UseAnticheatLvlUp", &iAnticheatLvlUp, 1);
		 bAnticheatLvlUp = (iAnticheatLvlUp != 0);
		 config->Read("StopLvlUpFirstViolation", &iACLvlUpStopFirstViolation, 1);
		 bACLvlUpStopFirstViolation = (iACLvlUpStopFirstViolation != 0);

		 config->Read("GrantedCondForEveryFeats", &iGrantedCondForEveryFeats, 0);
		 bGrantedCondForEveryFeats = (iGrantedCondForEveryFeats != 0);


		 logger->Debug("UseAnticheatLvlUp set to %s", bAnticheatLvlUp ? "TRUE" : "FALSE");

		 //Script on error ?
		 //config->Read("CallScriptOnLvlUpError", &iCallScriptOnLvlUpError, 0);
		 //bCallScriptOnLvlUpError = (iCallScriptOnLvlUpError != 0);
		 //if (bCallScriptOnLvlUpError)
		 {
			 config->Read("ScriptLvlUpError", &ScriptLvlUpError, std::string(""));
			 if (ScriptLvlUpError == "")
				 bCallScriptOnLvlUpError = false;
			 else
				 bCallScriptOnLvlUpError = true;
		 }
		 

		 std::string RangerCombatFeats = "";
		 config->Read("RangerCombatStyleFeats", &RangerCombatFeats, std::string("1729 1730"));

		 int num = 0;
		 std::istringstream iss(RangerCombatFeats);
		 while (iss >> num)
		 {
			 if (num < 0)
				 break;
			 lRangerCombatFeats.push_back(num);
		 }

		 if (bAnticheatLvlUp)
		 {
			 if (bACLvlUpStopFirstViolation)
				 logger->Debug("StopLvlUp at First Violation");
			 if (bGrantedCondForEveryFeats)
				 logger->Debug("GrantedCondition work for every Feats");
			 if (bCallScriptOnLvlUpError)
				 logger->Debug("Script on LevelUp error : %s", ScriptLvlUpError.c_str());
			 logger->Debug("Ranger CombatStyle Feats : %s", RangerCombatFeats.c_str());
		 }

	 }



	bool bIsLoadOk = true;

	if (LoadNetLayer())
		logger->Info("* BugFix Link successful");
	else
	{
		logger->Info("* Link failed");
		bIsLoadOk = false;
	}

	if (bIsLoadOk)
	{
		SetPacketFilterCallouts_(nullptr, nullptr, MsgServOnReceive, nullptr);
		logger->Info("* Plugin initialized.");

		g_msgServ = this;

		DWORD dwThreadId;
		HANDLE hThread;
		hThread = CreateThread(nullptr, 0, LaunchTestVersion, nullptr, 0, &dwThreadId);
	}

	return bIsLoadOk;
}


int version_compare(std::string v1, std::string v2)
{
	size_t i=0, j=0;
	while( i < v1.length() || j < v2.length() )
	{
		int acc1=0, acc2=0;

		while (i < v1.length() && v1[i] != '.') {  acc1 = acc1 * 10 + (v1[i] - '0');  i++;  }
		while (j < v2.length() && v2[j] != '.') {  acc2 = acc2 * 10 + (v2[j] - '0');  j++;  }

		if (acc1 < acc2)  return -1;
		if (acc1 > acc2)  return +1;

		++i;
		++j;
	}
	return 0;
}

bool LoadNetLayer()
{
	//
	// Wire up the dllimports.
	//

	struct { bool Required; const char *Name; void **Import; } DllImports[] =
	{
		{ true , "SendMessageToPlayer",     (void**)&SendMessageToPlayer_     },
		{ true , "GetPlayerAccountName",    (void**)&GetPlayerAccountName_    },
		{ true , "GetPlayerConnectionInfo", (void**)&GetPlayerConnectionInfo_ },
		{ true , "SetPacketFilterCallouts", (void**)&SetPacketFilterCallouts_ },
	};
	XPBugfix = LoadLibraryA("xp_bugfix.dll");

	if (!XPBugfix)
	{
		logger->Info("* Failed to load xp_bugfix.dll");
		return false;
	}

	for (int i = 0; i < sizeof(DllImports)/sizeof(DllImports[0]); i += 1)
	{
		*DllImports[i].Import = (void *)GetProcAddress(XPBugfix, DllImports[i].Name);

		if (!*DllImports[i].Import)
		{
			if (!DllImports[i].Required)
			{
				logger->Info(
					"* Warning: You need to update your xp_bugfix.dll; missing optional entrypoint xp_bugfix!%s",
					DllImports[i].Name);
				continue;
			}
			logger->Info("* Unable to resolve XPBugfix!%s", DllImports[i].Name);
			return false;
		}
	}

	return true;
}


int
MsgServ::GetInt(char* sFunction, [[maybe_unused]] char* sParam1, int nParam2)
{
	std::string function{sFunction};
	std::string logTxt =
		"MsgServ_GetInt(" + function + "," + sParam1 + "," + std::to_string(nParam2) + ")";

	logger->Trace(logTxt.c_str());

	return 0;
}

void MsgServ::SetInt([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2,
	[[maybe_unused]] int nValue)
{
	std::string function{sFunction};
	std::string logTxt =
		"MsgServ_SetInt(" + function + "," + sParam1 + "," + std::to_string(nParam2) + "," + std::to_string(nValue) + ")";

	logger->Trace(logTxt.c_str());

	if (function == "H_Enable") {
		m_heimdall.enabled  = (nValue != 0);
		logTxt = "Set Connection Process to : ";
		if (m_heimdall.enabled)
			logTxt += "TRUE";
		else
			logTxt += "FALSE";
		logger->Trace(logTxt.c_str());
	}
	else if (function == "AnticheatCreation")
	{
		ChangeCharacterCreationStatus((nValue != 0));
		logTxt = "Set AnticheatCreation System to : ";
		if (nValue != 0)
			logTxt += "TRUE";
		else
			logTxt += "FALSE";
		logger->Trace(logTxt.c_str());
	}
	else if (function == "AnticheatLvlUp")
	{
		bAnticheatLvlUp = (nValue != 0);
		logTxt = "Set AnticheatLevelup System to : ";
		if (bAnticheatLvlUp)
			logTxt += "TRUE";
		else
			logTxt += "FALSE";
		logger->Trace(logTxt.c_str());
	}
	else{
		logger->Err("Unknown %s command: %s", __FUNCTION__, sFunction);
	}

	return;
}


char*
MsgServ::GetString([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2)
{
	static std::string res;
	res = ProcessQueryFunction(std::string_view{sFunction});
	return const_cast<char*>(res.c_str());
}


std::tuple<std::string_view, std::string_view> splitSceneObjName(const std::string_view& text){
	const auto pos = text.find('>');
	if(pos != std::string::npos)
		return {text.substr(0, pos), text.substr(pos + 1)};
	return {text, ""};
}

void
MsgServ::SetString([[maybe_unused]] char* sFunction,
	[[maybe_unused]] char* sParam1,
	[[maybe_unused]] int nParam2,
	[[maybe_unused]] char* sValue)
{
	logger->Trace("MsgServ_SetString(%s, %s, %d, %s)", sFunction, sParam1, nParam2, sValue);

	std::string_view function{sFunction};
	if(function == "H_SetGUIScript"){
		NWNX_SetAuthorizedGUIScript(sValue);
	}
	else if(function == "H_DisplayGuiScreen"){
		NWNX_DisplayGUIScreen(sParam1, sValue);
	}
	else if(function == "H_CloseGUIScreen"){
		NWNX_CloseGUIScreen(sParam1);
	}
	else if(function == "H_SetGUIObjectHidden"){
		auto [scene, objectName] = splitSceneObjName(sParam1);
		if(!objectName.empty()){
			NWNX_SetGUIObjectHidden(scene, objectName, nParam2 != 0);
		} else{
			logger->Err("malformed scene+object name");
		}
	}
	else if(function == "H_SetGUIObjectText"){
		auto [scene, objectName] = splitSceneObjName(sParam1);
		if(!objectName.empty()){
			NWNX_SetGUIObjectText(scene, objectName, sValue);
		} else{
			logger->Err("malformed scene+object name");
		}
	}
	else{
		logger->Err("Unknown %s command: %s", __FUNCTION__, sFunction);
	}

	return;
}


void
MsgServ::GetFunctionClass(char* fClass)
{
	static constexpr auto cls = std::string_view(FunctionClass);
	strncpy_s(fClass, 128, cls.data(), std::size(cls));
}


void MsgServ::NWNX_SetAuthorizedGUIScript(const std::string_view& scriptName){
	if(!m_heimdall.currentPlayerInfo){
		logger->Err("Called " __FUNCTION__ " outside of the execution of the OnConnection script");
		return;
	}
	const auto& [playerId, accountName] = m_heimdall.currentPlayerInfo.value();
	m_heimdall.authorizedGUIScripts[playerId] = scriptName;
}

void MsgServ::NWNX_DisplayGUIScreen(const std::string_view& sceneName, const std::string_view& xmlName){
	if(!m_heimdall.currentPlayerInfo){
		logger->Err("Called " __FUNCTION__ " outside of the execution of the OnConnection script");
		return;
	}
	const auto& [playerId, accountName] = m_heimdall.currentPlayerInfo.value();
	SendOpenGUI(playerId, sceneName, xmlName);
}

void MsgServ::NWNX_CloseGUIScreen(const std::string_view& sceneName){
	if(!m_heimdall.currentPlayerInfo){
		logger->Err("Called " __FUNCTION__ " outside of the execution of the OnConnection script");
		return;
	}
	const auto& [playerId, accountName] = m_heimdall.currentPlayerInfo.value();
	SendCloseGUI(playerId, sceneName);
}

void MsgServ::NWNX_SetGUIObjectHidden(const std::string_view& sceneName, const std::string_view& objectName, bool hidden){
	if(!m_heimdall.currentPlayerInfo){
		logger->Err("Called " __FUNCTION__ " outside of the execution of the OnConnection script");
		return;
	}
	const auto& [playerId, accountName] = m_heimdall.currentPlayerInfo.value();
	SendSetGUIObjectHidden(playerId, sceneName, objectName, hidden);
}

void MsgServ::NWNX_SetGUIObjectText(const std::string_view& sceneName, const std::string_view& objectName, std::string_view text){
	if(!m_heimdall.currentPlayerInfo){
		logger->Err("Called " __FUNCTION__ " outside of the execution of the OnConnection script");
		return;
	}
	const auto& [playerId, accountName] = m_heimdall.currentPlayerInfo.value();
	SendSetGuiObjectText(playerId, sceneName, objectName, text);
}