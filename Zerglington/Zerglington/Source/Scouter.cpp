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
	Broodwar->sendText("Initializing Scouter");
	startLocations	= Broodwar->getStartLocations();
	unscouted		= Broodwar->getStartLocations();
	homeBase = Broodwar->self()->getStartLocation();
	unscouted.erase(homeBase);
	if (unscouted.size() == 1)
	{
		Broodwar->sendText("Only one base");
		foundBase(*unscouted.begin());
	}
}

void Scouter::addOverlord(Unit* overlord)
{
	if (unscouted.empty())
		return;
	if (firstOverlord == NULL)
		firstOverlord = overlord;
	setDestination(overlord);
	Broodwar->sendText("Added Overlord to scouts");
}

void Scouter::addZergling(Unit* zergling)
{
	if (unscouted.empty())
		return;
	setDestination(zergling);
	Broodwar->sendText("Added Zergling to scouts");
}

void Scouter::addAllZerglings(void)
{
	set<Unit*> allUnits = Broodwar->self()->getUnits();
	for (set<Unit*>::iterator i = allUnits.begin(); i != allUnits.end(); i++)
	{
		if ((*i)->getType().getID() == UnitTypes::Zerg_Zergling && scouts.find((*i)) == scouts.end())
		{
			Broodwar->sendText("Found new Zergling");
			addZergling(*i);
		}
	}
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

bool Scouter::isScout(Unit* unit)
{
	return scouts.find(unit) != scouts.end();
}

TilePosition Scouter::findFurthestUnscouted(Unit* unit)
{
	if (unscouted.empty())
		return homeBase;
	
	TilePosition furthest = *unscouted.begin();
	TilePosition unitPosition = unit->getTilePosition();
	
	for (TileSet::iterator i = unscouted.begin(); i != unscouted.end(); i++)
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
	
	TilePosition closest = *unscouted.begin();
	TilePosition unitPosition = unit->getTilePosition();
	
	for (TileSet::iterator i = unscouted.begin(); i != unscouted.end(); i++)
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
	TilePosition closest = homeBase;
	TilePosition unitPosition = unit->getTilePosition();
	
	for (TileSet::iterator i = startLocations.begin(); i != startLocations.end(); i++)
	{
		if ((*i).getDistance(unitPosition) < closest.getDistance(unitPosition) || closest == homeBase)
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
}

void Scouter::foundBuilding(Unit* unit)
{
	if (!foundEnemyBase && unit->getType().isBuilding() && unit->getPlayer()->isEnemy(Broodwar->self()))
	{
		foundBase(findNearestStart(unit));
	}
}

TilePosition Scouter::getEnemyBase(void)
{
	return enemyBase;
}

void Scouter::scoutKilled(Unit* unit)
{
	ScoutMap::iterator scout = scouts.find(unit);
	if (scout != scouts.end())
	{
		unscouted.insert(scout->second);
		scouts.erase(scout);
	}
}

void Scouter::setDestination(Unit* unit, bool nearest)
{
	ScoutMap::iterator scout = scouts.find(unit);
	TilePosition destination;
	if (unit->getType().isFlyer() || nearest)
	{
		destination = findNearestUnscouted(unit);
	}
	else
	{
		destination = findFurthestUnscouted(unit);
	}

	if (destination == homeBase)
	{
		scouts.erase(scout);
	}
	else
	{
		if (scout == scouts.end())
		{
			scouts.insert(ScoutPair(unit, destination));
			unscouted.erase(destination);
		}
		else
		{
			scout->second = destination;
			unscouted.erase(destination);
		}
	}
}

void Scouter::updateScouts(void)
{
	if (!foundEnemyBase)
	{
		for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			Unit* scout = i->first;
			if (Broodwar->isVisible(i->second))
			{
				setDestination(scout);
			}

			scout->move(Position(i->second));
		}
	}
	else
	{
		if (firstOverlord->isIdle())
		{
			firstOverlord->move(Position(enemyBase));
		}
	}
}