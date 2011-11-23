#include "Scouter.h"

using namespace BWAPI;
using namespace std;

vector<Unit*> scouts;
set<BWTA::BaseLocation*> startLocations;
set<BWTA::BaseLocation*> scouted;
list<BWTA::BaseLocation*> unscouted;
map<BWTA::BaseLocation*, Unit* > enroute;
BWTA::BaseLocation* enemyBase;

Scouter::Scouter(void)
{
	startLocations = BWTA::getStartLocations();
	unscouted.assign(startLocations.begin(), startLocations.end());
	enemyBase = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::addOverlord(Unit* overlord)
{
	scouts.push_back(overlord);
}

void Scouter::addZergling(Unit* zergling)
{
	scouts.push_back(zergling);
}

BWTA::BaseLocation* findNearestUnscouted(Unit* unit)
{
	list<BWTA::BaseLocation*>::iterator i = unscouted.begin();
	BWTA::BaseLocation* closest = (*i);
	Position unitPosition = unit->getPosition();
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i)->getPosition().getDistance(unitPosition) < closest->getPosition().getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

void Scouter::foundBase(BWTA::BaseLocation* base)
{
	enemyBase = base;
}

bool Scouter::foundEnemyBase()
{
	return enemyBase != NULL;
}

void Scouter::foundUnit(Unit* unit)
{
	BWTA::Region* enemyRegion = BWTA::getRegion(unit->getTilePosition());

	// Determine if enemy unit's location is the same region as a possible enemy starting location
	for (set<BWTA::BaseLocation*>::iterator i = startLocations.begin(); i != startLocations.end(); i++)
	{
		if ((*i)->getRegion() == enemyRegion)
		{
			foundBase(*i);
			break;
		}
	}
}

BWTA::Region* Scouter::getEnemyBase(void)
{
	if (foundEnemyBase())
		return enemyBase->getRegion();
	else
		return NULL;
}

void Scouter::updateScouts(void)
{
	if (!unscouted.empty())
	{
		for (vector<Unit*>::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			if ((*i)->isIdle())
			{
				BWTA::BaseLocation* nearest = findNearestUnscouted((*i));
				(*i)->move(nearest->getPosition());
				unscouted.remove(nearest);
				enroute.insert(pair<BWTA::BaseLocation*, Unit*>(nearest, (*i)));
			}
		}
	}
}