#pragma once
#include <BWAPI.h>
#include <vector>
#include <queue>

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
	BWAPI::Position muster;

private:
	std::vector<BWAPI::Unit*> strikers;
	BWAPI::Position enemyBase;
	std::deque<BWAPI::Unit*> targets;
};
