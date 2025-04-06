enum CE_ELootCategory
{
	//NONE = 				1 << -1,
	CLOTHING	=			1 << 0,
	//CONTAINERS =			1 << 1, // might remove, don't use
	//EXPLOSIVES =			1 << 2, // might remove, don't use
	FOOD =				1 << 3,
	TOOLS =				1 << 4,
	WEAPONS =			1 << 5,
};

enum CE_ELootUsage
{
	COAST =				1 << 0,
	FARM =				1 << 1,
	HUNTING =			1 << 2,
	INDUSTRIAL =			1 << 3,
	MEDIC =				1 << 4,
	MILITARY =			1 << 5,
	POLICE =				1 << 6,
	TOWN =				1 << 7,
};

enum CE_ELootTier
{
	TIER1 =				1 << 0,
	TIER2 =				1 << 1,
	TIER3 =				1 << 2,
	TIER4 =				1 << 3,
};