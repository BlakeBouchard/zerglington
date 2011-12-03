#pragma once
#include <BWAPI.h>
#include <map>
#include <set>
#include <vector>

// VEGETA. 
// VEGETA.
// VEGETA. 
// VEGETA.

extern bool foundEnemyBase;

class Scouter
{
public:

	Scouter(void);
	~Scouter(void);

	void initialize(void);
	void addOverlord(BWAPI::Unit*);
	void addZergling(BWAPI::Unit*);
	void addAllZerglings(void);
	void dumpZerglings(void);
	bool isScout(BWAPI::Unit*);
	void foundBase(BWAPI::TilePosition);
	void foundBuilding(BWAPI::Unit*);
	BWAPI::TilePosition getEnemyBase();
	void scoutKilled(BWAPI::Unit*);
	void setDestination(BWAPI::Unit*, bool nearest = false);
	void updateScouts(void);

	BWAPI::TilePosition findFurthestUnscouted(BWAPI::Unit*);
	BWAPI::TilePosition findNearestUnscouted(BWAPI::Unit*);
	BWAPI::TilePosition findNearestStart(BWAPI::Unit*);

	BWAPI::TilePosition homeBase;

private:
	typedef std::set<BWAPI::TilePosition> TileSet;
	typedef std::map<BWAPI::Unit*, BWAPI::TilePosition> ScoutMap;
	typedef std::pair<BWAPI::Unit*, BWAPI::TilePosition> ScoutPair;
	
	TileSet startLocations;
	TileSet unscouted;
	ScoutMap scouts;
	BWAPI::TilePosition enemyBase;
	BWAPI::Unit* firstOverlord;
};
