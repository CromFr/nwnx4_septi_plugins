#if !defined(SEPT_APPLISTMANAGEMENT)
#define SEPT_APPLISTMANAGEMENT

#include <string>
#include <misc/log.h> 


class AppearanceListManagement
{

protected :
	void analyser2da();
	void processZip();
	void processHak();
	void analyseSpecificIni();
	void parseFileName(std::string nom);
	void analyseZip(std::string zipPath);
	void Initialisation();
	bool processSaveFile();
	bool generateSaveFile();
	bool isSaveFileExist();


	void ListSystemSetInt(std::string sCommand, char* sParam1, int nParam2, int nValue);
	int ListSystemGetInt(std::string sCommand, char* sParam1, int nParam2);
	void ListSystemSetString(std::string sCommand, char* sParam1, int nParam2, char* sValue);
	std::string ListSystemGetString(std::string sCommand, char* sParam1, int nParam2);

	int armorPart(std::string race, std::string type, std::string part, std::string number);
	void addPresentationArmorList(std::string rrg,
		int iPart,
		std::string code,
		std::string visualPart);
	int getArmorPartCode(std::string part);
	int convertOldToNewModelPart(int iOldModelPartID);
	void accessoryPart(std::string race, std::string type, std::string number);
	void bodyPart(std::string race, std::string type, std::string number);
	bool DeleteCategorie(std::string Param1, int Param2, std::string Param3);
	void weapons(std::string type, std::string sousPartie, std::string number);
	bool CreateNewCategorie(std::string Param1, int Param2, std::string Param3);
	void addToTenue(std::string p1, int p2, std::string p3, unsigned int p4, std::string val);
	void removeFromTenue(std::string p1, int p2, std::string p3, unsigned int p4, std::string val);
	std::string CreateCodeApp(int iRCat, int iVar);
	std::string CreateCodeApp(std::string sRCat, std::string sVar);
	void removeFromCategory(std::string sRRG, std::string sCode, int iPart, int iRCat, int iVar, std::string sCategory);
	void addToCategory(std::string sRRG, std::string sCode, int iPart, int iRCat, int iVar, std::string sCategory);



protected :
	int GetNumberOfHairForRRG(char *sRRG);
	int GetNumberOfHeadForRRG(char *sRRG);

	int GetHairForRRG(char *sRRG, int iIdx);
	int GetHeadForRRG(char *sRRG, int iIdx);

	int GetNumberOfVisualCat(char* sRRG, int iArmorPart);
	int GetNumberOfArmorVariation(char* cRRG_Cat, int iArmorPart);

	int GetNumberOfArmorPieceVariation(char* cRRG, int iModelPiece);

	int GetArmorPiece(char* cRRG_Piece, int iIdx);

	int GetNumberOfWeaponSubPart(char* cWeaponType);
	int GetNumberOfVariationForWeaponSubPart(char* cWeapon, int iWpnPart);
	int GetWeaponPartVariation(char* cWeapon_SubPart, int iIdx);


	void SetRestrictedCategory(std::string sCategory, bool isRestricted);
	int GetRestrictedCategory(std::string sCategory);


	void RemoveAppearanceFromCategory(char* cRRG_RCat, int iPart_Var, char* cCategory);
	void RemoveAppearanceFromCategoryC(char* cRRG_sCode, int iPart, char* cCategory);

	void AddAppearanceToCategory(char* cRRG_RCat, int iPart_Var, char* cVCategory);
	void AddAppearanceToCategoryC(char* cRRG_sCode, int iPart, char* cCategory);

	void ChangeVisualCategory(char* cRRG_VisualCat, int iPart, char* cCode_NewVisualCat);

	void CreateNewVisualCategory(char* cRRG, int iPart, char* cVisualCat);
	void DeleteVisualCategory(char* cRRG, int iPart, char* cVisualCat);
	void RenameVisualCategory(char* cRRG_VisualCat, int iPart, char* cNewName);

	std::string GetVisualCategory(char* cRRG, int iPart_Idx);
	std::string GetVariationCode(char* cRRG_VCat, int iPart_Idx);

	int GetNumberOfVisualCategory(char *cRRG_Prefix, int iPart_Idx);
	std::string GetVisualCategoryOfRealAppearance(char* cRRG_Prefix, int iNum_Part_Idx);

	std::list<std::string> GetFullListOfVisualCategory(std::string sRace, std::string sGender, int iPart);
	int GetFullNumberOfVisualCategory(std::string sRace, std::string sGender, int iPart);
	std::string GetIndexOfFullListVisualCategory(std::string sRace, std::string sGender, int iPart, int iIdx);
	void ExtendedCopyVisualToAnother(std::string sRace, std::string sGender, int iPart, std::string sFromVisual, std::string toVisual);
	int DeleteExtendedVisualCategory(std::string sRace, std::string sGender, int iPart, std::string sVisualType);


public:
	LogNWNX* logger;

	std::string SEPARATOR_MODEL    = "_";
	std::string SAVEFILE_VERSION	= "1";

	std::list<std::string> listHak;
	std::list<std::string> listZip;

	std::string pathFileSave       = "";
	std::string pathAppFileSave	= "";
	std::string pathListFile = "";
	bool isNewFile              = false;
	int iScanFinish = 0;
	bool bScanFile = true;

public:

	std::unordered_map<std::string, int> prefixToIndex;
	std::unordered_map<int, std::string> indexToPrefix;
	//Used to automatically "Rename" an armorType
	std::unordered_map<std::string, std::string> prefixToVisual; 

public:

	//In the following, RRG is the code that represent the Subrace+Gender
	// Example : HHF for Human Female, EEM for Elf Male, etc.
	// RRG<iIdxModel>
	std::unordered_map<std::string, std::list<unsigned int>> m_hairs;

	// RRG<iIdxModel>
	std::unordered_map<std::string, std::list<unsigned int>> m_heads;

	// RRG< sArmorPiece < iIdxModel > >
	std::unordered_map<std::string, std::unordered_map<std::string, std::list<unsigned int>>>
		accessoriesMap;

	// WeaponType < sWeaponPart < iIdxModel > >
	std::unordered_map<std::string, std::unordered_map<std::string, std::list<unsigned int>>>
		weaponsMap;

	// RRG < iArmorPart < sCategoryPrefix < iIdxModel <sVisualCats> > > >
	std::unordered_map<
		std::string,
		std::unordered_map<
		int,
		std::unordered_map<std::string, std::unordered_map<unsigned int, std::vector<std::string>>>>>
		outfitMap;

	// RRG < iArmorPart < sVisualCat <  sCode
	std::unordered_map<
		std::string,
		std::unordered_map<int, std::unordered_map<std::string, std::list<std::string>>>>
		categorizedOutfitsMap;

	std::unordered_map<std::string, bool> restrictedCat;
	std::list<std::string> armorFileList;
};

#endif
