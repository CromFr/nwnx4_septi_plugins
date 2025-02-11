#if !defined(SEPT_ANTICHEAT)
#define SEPT_ANTICHEAT

int CheckForLevelUp(int playerId, const unsigned char* Data, size_t size, std::string sPlayerName, MsgServ* g_msgServ);
bool ApplyAntiCheatCreationPatch(SimpleIniConfig& config, bool bActivateIt);
void ChangeCharacterCreationStatus(bool bAntiCheatActivated);

#endif