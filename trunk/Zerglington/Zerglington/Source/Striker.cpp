#include "Striker.h"

using namespace BWAPI;
using namespace std;

Striker::Striker(void)
{
	initialized = false;
}

Striker::~Striker(void)
{
}

void Striker::initialize(TilePosition base)
{
	addAllZerglings();
	setEnemyBase(Position(base));
	initialized = true;
}

void Striker::addZergling(Unit* unit)
{
	strikers.push_back(unit);
}

void Striker::addAllZerglings(void)
{
	strikers.clear();
	set<Unit*> allUnits = Broodwar->getAllUnits();
	for (set<Unit*>::iterator i = allUnits.begin(); i != allUnits.end(); i++)
	{
		if ((*i)->getPlayer() == Broodwar->self() && (*i)->getType().getID() == UnitTypes::Zerg_Zergling)
			strikers.push_back(*i);
	}
}

void Striker::setEnemyBase(Position base)
{
	enemyBase = base;
}

void Striker::updateStrikers(void)
{
	if (initialized)
	{
		for (vector<Unit*>::iterator i = strikers.begin(); i != strikers.end(); i++)
		{
			if ((*i)->isIdle())
				(*i)->attack(enemyBase);
		}
	}
}