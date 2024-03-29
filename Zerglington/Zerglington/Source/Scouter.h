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
	void resetScouter(void);
	
	void addOverlord(BWAPI::Unit*);
	void addZergling(BWAPI::Unit*);
	void addAllZerglings(void);
	void dumpZerglings(void);

	bool isScout(BWAPI::Unit*);
	BWAPI::TilePosition getEnemyBase();

	void foundBase(BWAPI::TilePosition);
	void foundBuilding(BWAPI::Unit*);
	void scoutKilled(BWAPI::Unit*);
	
	void updateScouts(void);

	BWAPI::TilePosition findFurthestUnscouted(BWAPI::Unit*);
	BWAPI::TilePosition findNearestUnscouted(BWAPI::Unit*);
	BWAPI::TilePosition findNearestStart(BWAPI::Unit*);

	BWAPI::TilePosition homeBase;

private:
	void setDestination(BWAPI::Unit*, bool nearest = false);
	void setUnscouted(void);

	typedef std::set<BWAPI::TilePosition> TileSet;
	typedef std::map<BWAPI::Unit*, BWAPI::TilePosition> ScoutMap;
	typedef std::pair<BWAPI::Unit*, BWAPI::TilePosition> ScoutPair;
	
	TileSet startLocations;
	TileSet unscouted;
	ScoutMap scouts;
	BWAPI::TilePosition enemyBase;
	BWAPI::Unit* firstOverlord;
};
