#include "Scouter.h"

using namespace BWAPI;
using namespace std;

bool initialized;
vector<Unit*> scouts;
set<BWTA::BaseLocation*> startLocations;
set<BWTA::BaseLocation*> unscouted;
map<BWTA::BaseLocation*, Unit* > enroute;
BWTA::BaseLocation* enemyBase;

Scouter::Scouter(void)
{
	initialized = false;
	enemyBase = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::initialize(void)
{
	startLocations = BWTA::getStartLocations();
	unscouted = startLocations;
	Broodwar->printf("Found %d start locations", startLocations.size());
	initialized = true;
}

void Scouter::addOverlord(Unit* overlord)
{
	scouts.push_back(overlord);
	Broodwar->sendText("Added Overlord to scouts");
}

void Scouter::addZergling(Unit* zergling)
{
	scouts.push_back(zergling);
	Broodwar->sendText("Added Zergling to scouts");
}

BWTA::BaseLocation* findNearestUnscouted(Unit* unit)
{
	set<BWTA::BaseLocation*>::iterator i = unscouted.begin();
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
	Broodwar->sendText("Found enemy base");
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
	if (!initialized)
		return;

	if (!unscouted.empty())
	{
		for (vector<Unit*>::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			if ((*i)->isIdle())
			{
				BWTA::BaseLocation* nearest = findNearestUnscouted((*i));
				(*i)->move(nearest->getPosition());
				unscouted.erase(nearest);
				enroute.insert(pair<BWTA::BaseLocation*, Unit*>(nearest, (*i)));
				Broodwar->sendText("Scout moving to position");
			}
		}
	}
	if (!enroute.empty())
	{
		for (map<BWTA::BaseLocation*, Unit*>::iterator i = enroute.begin(); i != enroute.end(); i++)
		{
			if (i->first->getPosition() == i->second->getPosition())
			{
				enroute.erase(i);
			}
		}
	}
	if (!foundEnemyBase() && enroute.empty() && unscouted.empty())
	{
		Broodwar->sendText("WARNING: No more BaseLocations to scout, no bases found");
	}
}