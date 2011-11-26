#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <set>
#include <vector>
#include <windows.h>

// VEGETA. 
// VEGETA.
// VEGETA. 
// VEGETA.

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
	void drawTerrainData();
	void foundBase(BWTA::BaseLocation*);
	bool foundEnemyBase(void);
	void foundUnit(BWAPI::Unit*);
	BWTA::Region* getEnemyBase();
	void updateScouts(void);
};
