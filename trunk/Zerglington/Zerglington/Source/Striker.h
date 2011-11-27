#pragma once
#include <BWAPI.h>
#include <vector>

class Striker
{
public:
	Striker(void);
	~Striker(void);

	void initialize(BWAPI::TilePosition);
	void addZergling(BWAPI::Unit*);
	void addAllZerglings(void);
	void setEnemyBase(BWAPI::Position);
	void updateStrikers(void);

	bool initialized;

private:
	std::vector<BWAPI::Unit*> strikers;
	BWAPI::Position enemyBase;
};
