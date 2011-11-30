#include "Scouter.h"

using namespace BWAPI;
using namespace std;

bool foundEnemyBase;

Scouter::Scouter(void)
{
	firstOverlord = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::initialize(void)
{
	homeBase = Broodwar->self()->getStartLocation();
	startLocations = Broodwar->getStartLocations();
	unscouted = startLocations;
	unscouted.erase(homeBase);
	if (unscouted.size() == 1)
	{
		foundBase(*unscouted.begin());
	}
}

void Scouter::addOverlord(Unit* overlord)
{
	if (unscouted.empty())
		return;
	if (firstOverlord == NULL)
		firstOverlord = overlord;
	TilePosition destination = findNearestUnscouted(overlord);
	scouts.insert(ScoutPair(overlord, destination));
	unscouted.erase(destination);
	Broodwar->sendText("Added Overlord to scouts");
}

void Scouter::addZergling(Unit* zergling)
{
	if (unscouted.empty())
		return;
	TilePosition destination = findFurthestUnscouted(zergling);
	scouts.insert(ScoutPair(zergling, destination));
	unscouted.erase(destination);
	Broodwar->sendText("Added Zergling to scouts");
}

void Scouter::dumpZerglings(void)
{
	for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
	{
		if (i->first->getType().getID() == UnitTypes::Zerg_Zergling)
		{
			scouts.erase(i);
		}
	}
}

TilePosition Scouter::findFurthestUnscouted(Unit* unit)
{
	if (unscouted.empty())
		return homeBase;
	TileSet::iterator i = unscouted.begin();
	TilePosition furthest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i).getDistance(unitPosition) > furthest.getDistance(unitPosition))
		{
			furthest = (*i);
		}
	}

	return furthest;
}

TilePosition Scouter::findNearestUnscouted(Unit* unit)
{
	if (unscouted.empty())
		return homeBase;
	TileSet::iterator i = unscouted.begin();
	TilePosition closest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i).getDistance(unitPosition) < closest.getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

TilePosition Scouter::findNearestStart(Unit* unit)
{
	TileSet::iterator i = startLocations.begin();
	TilePosition closest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (; i != startLocations.end(); i++)
	{
		if ((*i).getDistance(unitPosition) < closest.getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

void Scouter::foundBase(TilePosition basePosition)
{
	enemyBase = basePosition;
	foundEnemyBase = true;
	Broodwar->sendText("Found enemy base");
	dumpZerglings();
}

void Scouter::foundUnit(Unit* unit)
{
	UnitType unitType = unit->getType();
	if (!foundEnemyBase && unitType.isBuilding() && !unitType.isNeutral())
	{
		foundBase(unit->getTilePosition());
	}
}

TilePosition Scouter::getEnemyBase(void)
{
	return enemyBase;
}

void Scouter::updateScouts(void)
{
	if (!foundEnemyBase)
	{
		for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			Unit* scout = i->first;
			Position destination = Position(i->second);
			if (scout->isUnderAttack())
			{
				if (scout->getType().isFlyer())
				{
					TilePosition nearest = findNearestUnscouted(scout);
					unscouted.insert(i->second);
					scout->move(Position(nearest));
					i->second = nearest;
				}
				else
				{
					TilePosition furthest = findFurthestUnscouted(scout);
					unscouted.insert(i->second);
					scout->move(Position(furthest));
					i->second = furthest;
				}
			}
			else if (Broodwar->isVisible(i->second))
			{
				i->second = findNearestUnscouted(scout);
				scout->move(Position(i->second));
			}
			else
			{
				scout->move(Position(i->second));
			}
		}
	}
	else
	{
		if (firstOverlord->isIdle())
			firstOverlord->move(Position(enemyBase));
	}
}