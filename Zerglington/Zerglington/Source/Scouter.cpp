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
	startLocations	= Broodwar->getStartLocations();
	homeBase = Broodwar->self()->getStartLocation();
	setUnscouted();
	Broodwar->sendText("Scouter Initialized");
}

void Scouter::resetScouter(void)
{
	setUnscouted();
	addAllZerglings();
}

void Scouter::setUnscouted(void)
{
	// All start locations except for homeBase are potential enemy bases
	unscouted = Broodwar->getStartLocations();
	unscouted.erase(homeBase);
}

void Scouter::addOverlord(Unit* overlord)
{
	if (unscouted.empty())
		return;
	if (firstOverlord == NULL)
		// We will use this overlord to spy on enemy base after striker has initialized
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
	// Adds all Zerglings to scouts
	// Detects if Zerglings are already in scouts, so that duplicate entries are not created
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
	// Removes all zerglings from scouts
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
	// Finds the furthest unscouted start location from selected unit
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
	// Finds the nearest unscouted start location from selected unit
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
	// Finds the nearest start locations from the selected unit
	TilePosition closest = homeBase;
	TilePosition unitPosition = unit->getTilePosition();
	
	for (TileSet::iterator i = startLocations.begin(); i != startLocations.end(); i++)
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
	// Enemy base has been found
	enemyBase = basePosition;
	dumpZerglings();
	foundEnemyBase = true;
	Broodwar->sendText("Found enemy base");
}

void Scouter::foundBuilding(Unit* unit)
{
	// Checks to determine whether unit is an enemy building, if so then an enemy base has been found
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
	// If a scout is killed before reaching its destination then its assignment is set as unscouted
	ScoutMap::iterator scout = scouts.find(unit);
	if (scout != scouts.end())
	{
		unscouted.insert(scout->second);
		scouts.erase(scout);
	}
}

void Scouter::setDestination(Unit* unit, bool nearest)
{
	// Finds a suitable unscouted destination for the selected unit, or if none exists, removes that
	//	 unit from the list of scouts
	ScoutMap::iterator scout = scouts.find(unit);
	TilePosition destination;

	if (unit->getType().isFlyer() || nearest)
	{
		// Overlords go to the nearest unscouted destination
		destination = findNearestUnscouted(unit);
	}
	else
	{
		// Zerglings go to the furthest unscouted destination
		destination = findFurthestUnscouted(unit);
	}

	// findNearest/FurthestUnscouted return homeBase if no suitable start location is found
	if (destination == homeBase) 
	{
		// If a scout cannot be assigned to a suitable destination, it is removed from the list of
		//	 scouts along with its assignment
		scouts.erase(scout);
	}
	else
	{
		if (scout == scouts.end()) // Iterator reached end, unit was not found in scouts list
		{
			// Unit is a new scout, add to scouts list
			scouts.insert(ScoutPair(unit, destination));
			// Remove destination from unscouted so no other scout attempts to search it
			unscouted.erase(destination);
		}
		else // Iterator found unit in scouts list
		{
			// Unit is already a scout, reassign destination
			scout->second = destination;
			// Remove destination from unscouted so no other scout attempts to search it
			unscouted.erase(destination);
		}
	}
}

void Scouter::updateScouts(void)
{
	if (!foundEnemyBase)
	{
		if (unscouted.empty() && scouts.empty())
		{
			// If the enemy base has not been found and there are no locations left to check, start over
			// This may happen if striker runs out of targets and an initial sweep reveals no new enemies
			resetScouter();
		}

		if (unscouted.empty() && scouts.size() == 1)
		{
			// If there is only one location left to scout and it already has a scout assigned to it,
			//   then that location must be the enemy base
			foundBase(scouts.begin()->second);
		}

		for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			Unit* scout = i->first;
			if (Broodwar->isVisible(i->second))
			{
				// If scout's destination is visible, then either:
				//   1. AIModule will call onUnitShow on a building at the start location, which will
				//		determine that there is a start location at that point
				//	 2.	No buildings will be spotted at start location, so scout will be reassigned
				setDestination(scout, true);
			}

			// Send scout to its destination
			// Destination is part of scout map, with the scout itself acting as the key
			scout->move(Position(i->second));
		}
	}
	else
	{
		if (firstOverlord->isIdle())
		{
			// Overlord will hover over enemy base to scout for strikers
			firstOverlord->move(Position(enemyBase));
		}
	}
}