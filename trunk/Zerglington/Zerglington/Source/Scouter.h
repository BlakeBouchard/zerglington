#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <map>
#include <set>
#include <vector>
#include <windows.h>

// VEGETA. 
// VEGETA.
// VEGETA. 
// VEGETA.

extern bool foundEnemyBase;
extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class Scouter
{
public:

	Scouter(void);
	~Scouter(void);

	void initialize(void);
	void addOverlord(BWAPI::Unit*);
	void addZergling(BWAPI::Unit*);
	void dumpZerglings(void);
	void drawTerrainData();
	void foundBase(BWAPI::TilePosition);
	void foundUnit(BWAPI::Unit*);
	BWAPI::TilePosition getEnemyBase();
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
