#include "Striker.h"

using namespace BWAPI;
using namespace std;

Striker::Striker(void)
{
	initialized = false;
	foundPriority = false;
}

Striker::~Striker(void)
{
}

void Striker::initialize(TilePosition base)
{
	addAllZerglings();
	setEnemyBase(Position(base));
	setEnemyPlayer();
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
		{
			strikers.push_back(*i);
		}
	}
}

Unit* Striker::findNearestToMuster(set<Unit*> enemyUnits)
{
	if (enemyUnits.empty())
	{
		return NULL;
	}

	set<Unit*>::iterator i = enemyUnits.begin();
	Unit* closest = *i;
	for (++i; i != enemyUnits.end(); i++)
	{
		if ((*i)->getDistance(muster) < closest->getDistance(muster))
		{
			closest = *i;
		}
	}

	return closest;
}

void Striker::setEnemyBase(Position base)
{
	enemyBase = base;
}

void Striker::setEnemyPlayer()
{
	set<Player*> players = Broodwar->getPlayers();
	for (set<Player*>::iterator i = players.begin(); i != players.end(); i++)
	{
		if ((*i)->isEnemy(Broodwar->self()))
		{
			enemyPlayer = *i;
			return;
		}
	}
}

void Striker::setMuster(void)
{
	BWTA::Chokepoint* chokepoint = BWTA::getNearestChokepoint(enemyBase);
	if (chokepoint != NULL)
	{
		muster = chokepoint->getCenter();
	}
	else
	{
		int centerX = Broodwar->mapWidth()  / 2;
		int centerY = Broodwar->mapHeight() / 2;
		int baseX = enemyBase.x();
		int baseY = enemyBase.y();

		muster = Position((baseX - centerX) / 2 + centerX, (baseY - centerY) / 2 + centerY);
	}

	foundMuster = true;
}

void Striker::setTarget(void)
{
	set<Unit*> enemyUnits = enemyPlayer->getUnits();
	if (enemyUnits.empty())
		return;

	set<Unit*> workers;
	set<Unit*> attackers;
	bool foundWorker = false;
	bool foundAttacker = false;

	for (set<Unit*>::iterator i = enemyUnits.begin(); i != enemyUnits.end(); i++)
	{
		if ((*i)->getType().isWorker())
		{
			workers.insert((*i));
			foundWorker = true;
		} 
		else if (!foundWorker && (*i)->getType().canAttack())
		{
			attackers.insert((*i));
			foundAttacker = true;
		}
	}

	if (foundWorker)
	{
		target = findNearestToMuster(workers);
		foundPriority = true;
	}
	else if (foundAttacker)
	{
		target = findNearestToMuster(attackers);
		foundPriority = true;
	}
	else
	{
		muster = enemyBase;
		foundMuster = true;
		target = findNearestToMuster(enemyUnits);
	}
}

void Striker::updateStrikers(void)
{
	if (!initialized)
	{
		return;
	}

	if (!foundMuster)
	{
		setMuster();
	}

	if (!target->isVisible())
	{
		foundPriority = false;
		setTarget();
	}

	for (vector<Unit*>::iterator i = strikers.begin(); i != strikers.end(); i++)
	{
		if (foundPriority && target->isVisible())
		{
			(*i)->attack(target);
		}
		else
		{
			(*i)->attack(enemyBase);
		}
	}
}