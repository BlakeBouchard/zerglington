#include "Striker.h"

using namespace BWAPI;
using namespace std;

Striker::Striker(void)
{
	initialized = false;
	foundMuster = false;
	noMoreUnits = false;
}

Striker::~Striker(void)
{
}

void Striker::initialize(TilePosition base)
{
	// Initializes Striker by setting the enemy base and keeping references to all available Zerglings
	setEnemyBase(Position(base));
	addAllZerglings();
	noMoreUnits = false;
	initialized = true;
	Broodwar->sendText("Striker initialized");
}

void Striker::addZergling(Unit* unit)
{
	// If Zergling is not already in Strikers, add it
	if (!isStriker(unit))
	{
		strikers.insert(unit);
	}
}

void Striker::addAllZerglings(void)
{
	// Iterate through all friendly units and add the Zerglings to internal storage
	set<Unit*> allUnits = Broodwar->self()->getUnits();
	for (set<Unit*>::iterator i = allUnits.begin(); i != allUnits.end(); i++)
	{
		if ((*i)->getType().getID() == UnitTypes::Zerg_Zergling)
		{
			addZergling(*i);
		}
	}
}

bool Striker::isStriker(Unit* unit)
{
	
	return strikers.find(unit) != strikers.end();
}

//Unit* Striker::findNearestToMuster(set<Unit*> enemyUnits)
//{
//	if (!foundMuster || enemyUnits.empty())
//	{
//		return NULL;
//	}
//
//	set<Unit*>::iterator i = enemyUnits.begin();
//	Unit* closest = *i;
//	for (++i; i != enemyUnits.end(); i++)
//	{
//		if ((*i)->getDistance(muster) < closest->getDistance(muster))
//		{
//			closest = *i;
//		}
//	}
//
//	return closest;
//}

Unit* Striker::findNearestUnit(Unit* unit, set<Unit*> unitSet)
{
	// Returns the nearest unit to the first parameter from the set of units in the second parameter
	if (unitSet.empty())
	{
		// No units to return
		return NULL;
	}

	set<Unit*>::iterator i = unitSet.begin();
	Unit* closest = *i;
	for (++i; i != unitSet.end(); i++)
	{
		if ((*i)->getDistance(unit) < closest->getDistance(unit))
		{
			closest = *i;
		}
	}

	return closest;
}

void Striker::setEnemyBase(Position base)
{
	enemyBase = base;
	// Sets the enemy base as the initial position for the strikers to attack
	targetPosition = enemyBase;
}

//void Striker::setEnemyPlayer()
//{
//	// NOT USED
//	set<Player*> players = Broodwar->getPlayers();
//	for (set<Player*>::iterator i = players.begin(); i != players.end(); i++)
//	{
//		if ((*i)->isEnemy(Broodwar->self()))
//		{
//			enemyPlayer = *i;
//			return;
//		}
//	}
//}

//void Striker::setMuster(void)
//{
//	BWTA::Chokepoint* chokepoint = BWTA::getNearestChokepoint(enemyBase);
//	if (chokepoint != NULL)
//	{
//		muster = chokepoint->getCenter();
//	}
//	else
//	{
//		int centerX = Broodwar->mapWidth()  / 2;
//		int centerY = Broodwar->mapHeight() / 2;
//		int baseX = enemyBase.x();
//		int baseY = enemyBase.y();
//
//		muster = Position((baseX - centerX) / 2 + centerX, (baseY - centerY) / 2 + centerY);
//	}
//
//	foundMuster = true;
//	Broodwar->sendText("Muster set successfully");
//}

void Striker::setTarget(void)
{
	// Calls getShownTarget to get a unit target
	if (strikers.empty())
	{
		// No strikers to use
		return;
	}

	target = getShownTarget(*strikers.begin());

	if (target != NULL)
	{
		hidden.push(targetPosition);
		targetPosition = target->getPosition();
	}
}

Unit* Striker::getShownTarget(Unit* unit)
{
	if (shown.empty())
	{
		// No visible enemy units so return
		return NULL;
	}

	set<Unit*> workers;
	set<Unit*> attackers;
	set<Unit*> baseDefense;
	set<Unit*> benign;

	// Iterate through all shown units and add them to a specific set depending on their type
	for (set<Unit*>::iterator i = shown.begin(); i != shown.end(); i++)
	{
		UnitType type = (*i)->getType();
		if (type.isWorker())
		{
			// Resource gatherer/builder
			workers.insert(*i);
		}
		else if (type.canAttack())
		{
			if (type.isBuilding())
			{
				// Base Defese
				baseDefense.insert(*i);
			}
			else
			{
				// Mobile Attacker
				attackers.insert(*i);
			}
		}
		else
		{
			// Everything that can't attack or gather resources
			benign.insert(*i);
		}
	}

	// Prioritizes units based on whether they can attack, if they are buildings, or if they are workers
	if (!attackers.empty())
	{
		return findNearestUnit(unit, attackers);
	}
	else if (!baseDefense.empty())
	{
		return findNearestUnit(unit, baseDefense);
	}
	else if (!workers.empty())
	{
		return findNearestUnit(unit, workers);
	} 
	else if (!benign.empty())
	{
		return findNearestUnit(unit, benign);
	}
	else
	{
		return NULL;
	}
}

void Striker::checkAllTargets(void)
{
	// Resets the shown container to all visible enemy units
	Broodwar->sendText("Checking all targets");
	set<Unit*> allUnits = Broodwar->getAllUnits();
	shown.clear();
	for (set<Unit*>::iterator i = allUnits.begin(); i != allUnits.end(); i++)
	{
		if ((*i)->getPlayer()->isEnemy(Broodwar->self()) && !(*i)->getType().isFlyer())
		{
			shown.insert(*i);
		}
	}

	if (shown.empty() && hidden.empty() && target == NULL)
	{
		initialized = false;
		noMoreUnits = true;
	}
	else
	{
		setTarget();
	}
}

void Striker::unitDiscovered(BWAPI::Unit* unit)
{
	// If CompleteMapInformation is disabled, this is functionally equivalent to unitShown
	unitShown(unit);
}

void Striker::unitKilled(BWAPI::Unit* unit)
{
	if (isStriker(unit))
	{
		// Unit killed was a friendly zergling, remove its reference from list
		strikers.erase(unit);
	}
	else
	{
		// Remove unit from visible units, reassign targets
		target = NULL;
		shown.erase(unit);
		setTarget();
	}
}

void Striker::unitHidden(BWAPI::Unit* unit)
{
	if (unit->getType().isFlyer())
	{
		// Ignore flying units
		return;
	}

	shown.erase(unit);
	// Store hidden unit's position so that it can be attacked in the future
	hidden.push(unit->getPosition());

	if (unit == target)
	{
		// Unit was previous target, choose new target
		target = NULL;
		setTarget();
	}
}

void Striker::unitShown(BWAPI::Unit* unit)
{	
	if (unit->getType().isFlyer())
	{
		// Ignore flying units
		return;
	}

	if (shown.find(unit) == shown.end() && unit->getPlayer()->isEnemy(Broodwar->self()))
	{
		// Unit not already in shown
		shown.insert(unit);
	}

	// New target found so determine what to attack based on new information
	setTarget();
}

void Striker::attackTarget(void)
{
	for (set<Unit*>::iterator i = strikers.begin(); i != strikers.end(); i++)
	{
		if (!(*i)->isAttacking() && !(*i)->isMoving())
		{
			// Zergling is not already attacking or moving to a location
			if ((*i)->isUnderAttack() && !target->getType().canAttack())
			{
				// Zergling is under attack and target is not an attacker, so reassign
				setTarget();
			}
			(*i)->attack(target);
		}
	}
}

void Striker::attackPosition(void)
{
	// Assigns a position for all available strikers to attack
	// Should only be called if no units are visible
	set<Unit*> tileUnits = Broodwar->getUnitsOnTile(targetPosition.x(), targetPosition.y());
	if (Broodwar->isVisible(TilePosition(targetPosition)))
	{
		for (set<Unit*>::iterator i = tileUnits.begin(); i != tileUnits.end(); i++)
		{
			if (!(*i)->getPlayer()->isEnemy(Broodwar->self()))
			{
				tileUnits.erase(i);
			}
		}
		if (tileUnits.empty() && !hidden.empty())
		{
			targetPosition = hidden.front();
			hidden.pop();
		}
		else
		{
			setTarget();
		}
	}
	else
	{
		Broodwar->sendText("Attacking target position");
		for (set<Unit*>::iterator i = strikers.begin(); i != strikers.end(); i++)
		{
			(*i)->attack(targetPosition);
		}
	}
}

void Striker::updateStrikers(void)
{
	if (!initialized || strikers.empty())
	{
		return;
	}

	/*if (false && !foundMuster)
	{
		Broodwar->sendText("No muster set");
		setMuster();
	}*/

	if (shown.empty() && hidden.empty())
	{
		// No visible units and no locations to attack, so check to see if there is anything missing
		checkAllTargets();
	}

	if (target == NULL)
	{
		// No valid target to attack, so send units to one of the last known enemy positions
		attackPosition();
	} 
	else
	{
		// Target is a valid, visible unit, so send all zerglings to attack
		attackTarget();
	}
}

// Private member functions
bool Striker::isVisible(Unit* unit)
{
	return shown.find(unit) != shown.end();
}