#include "Scouter.h"

using namespace BWAPI;
using namespace std;

vector<Unit*> overlords;
vector<Unit*> zerglings;
set<BWTA::BaseLocation*> startLocations;

BWTA::BaseLocation* enemyBase;

Scouter::Scouter()
{
	startLocations = BWTA::getStartLocations();
	enemyBase = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::addOverlord(Unit* overlord)
{
	overlords.push_back(overlord);
}

void Scouter::addZergling(Unit* zergling)
{
	zerglings.push_back(zergling);
}

void Scouter::foundBase(BWTA::BaseLocation* base)
{
	enemyBase = base;
}

void Scouter::foundBase(BWTA::Region* region)
{
	for (set<BWTA::BaseLocation*>::iterator i = startLocations.begin(); i != startLocations.end(); i++)
	{
		if ((*i)->getRegion() == region)
		{
			enemyBase = (*i);
			break;
		}
	}
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

BWTA::Region* Scouter::getEnemyBase()
{
	if (foundEnemyBase())
		return enemyBase->getRegion();
	else
		return NULL;
}

void Scouter::updateScouts(void)
{

}