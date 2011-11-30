#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <set>

class Striker
{
public:
	Striker(void);
	~Striker(void);

	void initialize(BWAPI::TilePosition);
	void addZergling(BWAPI::Unit*);
	void addAllZerglings(void);
	BWAPI::Unit* findNearestToMuster(std::set<BWAPI::Unit*>);
	void setEnemyBase(BWAPI::Position);
	void setEnemyPlayer(void);
	void setMuster(void);
	void setTarget(void);
	void updateStrikers(void);

	bool initialized;

private:

	std::vector<BWAPI::Unit*> strikers;
	BWAPI::Position enemyBase;
	BWAPI::Player* enemyPlayer;
	
	bool foundPriority;
	BWAPI::Unit* target;

	bool foundMuster;
	BWAPI::Position muster;
};
