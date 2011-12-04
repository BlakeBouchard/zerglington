#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <queue>

class Striker
{
public:
	Striker(void);
	~Striker(void);

	void initialize(BWAPI::TilePosition);
	void addZergling(BWAPI::Unit*);
	void addAllZerglings(void);
	void attackTarget(void);
	void attackPosition(void);
	bool isStriker(BWAPI::Unit*);
	BWAPI::Unit* findNearestToMuster(std::set<BWAPI::Unit*>);
	BWAPI::Unit* findNearestUnit(BWAPI::Unit*, std::set<BWAPI::Unit*>);
	void setEnemyBase(BWAPI::Position);
	void setEnemyPlayer(void);
	void setMuster(void);
	void setTarget(void);
	BWAPI::Unit* getShownTarget(BWAPI::Unit* unit);
	void checkAllTargets(void);
	void unitDiscovered(BWAPI::Unit* unit);
	void unitKilled(BWAPI::Unit* unit);
	void unitHidden(BWAPI::Unit* unit);
	void unitShown(BWAPI::Unit* unit);
	void updateStrikers(void);

	bool initialized;
	bool noMoreUnits;

private:

	
	BWAPI::Position enemyBase;
	BWAPI::Player* enemyPlayer;
	BWAPI::Unit* target;
	BWAPI::Position targetPosition;
	
	std::set<BWAPI::Unit*> strikers;
	std::set<BWAPI::Unit*> shown;
	std::queue<BWAPI::Position> hidden;

	bool isVisible(BWAPI::Unit* unit);

	bool foundMuster;
	BWAPI::Position muster;
};
