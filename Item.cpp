#include <iostream>
#include <map>

using std::string;
using std::map;

namespace item 
{
	enum StatType 
	{
		STRENGTH = 0,
		DEXTERITY = 1,
		POWER = 2,
		KNOWLEDGE = 3,
		HEALTH = 4,
		MANA = 5,
		STAMINA = 6,
		SLASHING = 7,
		CRUSHING = 8,
		PIERCING = 9,
		COLD = 10,
		FIRE = 11,
		ENERGY = 12,
		MENTAL = 13
	};

	enum RankType 
	{
		I = 0,
		II = 1,
		III = 2,
		IV = 3,
		V = 4,
		VI = 5,
		VII = 6,
		VIII = 7,
		IX = 8,
		X = 9
	};

	enum RarityType 
	{
		COMMON = 1250,
		UNCOMMON = 2500,
		SET = 4000,
		RARE = 5000,
		LEGENDARY = 7500,
		ANCIENT = 7500
	};

	enum ItemType
	{
		HELMET = 0,
		NECKLACE = 1,
		GLOVES = 2,
		RING = 3,
		BRACERS = 4,
		SHIELD = 5,
		BOOTS = 6,
		PANTS = 7,
		BELT = 8,
		ARMOUR = 9,
		CAPE = 10,
		BOW = 11,
		AXE = 12,
		AXE_TH = 13,
		SWORD = 14,
		SWORD_TH = 15,
		KNUCKLES = 16,
		STAFF = 17
	};

	struct Item
	{
		int id;
		string name;
		int level;
		ItemType type;
		RankType rank;
		RarityType rarity;
		map<StatType, int> stats;
		map<StatType, int> requirements;
	};
}
