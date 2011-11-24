#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <set>
#include <vector>

// VEGETA. 
// VEGETA.
// VEGETA. 
// VEGETA.

class Scouter
{
public:
	Scouter(void);
	~Scouter(void);

	void addOverlord(BWAPI::Unit*);
	void addZergling(BWAPI::Unit*);
	void foundBase(BWTA::BaseLocation*);
	bool foundEnemyBase(void);
	void foundUnit(BWAPI::Unit*);
	BWTA::Region* getEnemyBase();
	void updateScouts(void);
};
