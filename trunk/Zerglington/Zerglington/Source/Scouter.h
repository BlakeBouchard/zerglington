#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <set>
#include <vector>

// VEGETA. VEGETA. VEGETA. VEGETA.

class Scouter
{
public:
	Scouter();
	~Scouter(void);

	void addOverlord(BWAPI::Unit*);
	void addZergling(BWAPI::Unit*);
	void foundUnit(BWAPI::Unit*);
	BWTA::Region* updateScouts(void);

private:
	
};
