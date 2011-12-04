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
	setEnemyBase(Position(base));
	addAllZerglings();
	noMoreUnits = false;
	initialized = true;
	Broodwar->sendText("Striker initialized");
}

void Striker::addZergling(Unit* unit)
{
	strikers.insert(unit);
}

void Striker::addAllZerglings(void)
{
	strikers.clear();
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

Unit* Striker::findNearestToMuster(set<Unit*> enemyUnits)
{
	if (!foundMuster || enemyUnits.empty())
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

Unit* Striker::findNearestUnit(Unit* unit, set<Unit*> unitSet)
{
	if (unitSet.empty())
	{
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
	targetPosition = enemyBase;
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
	Broodwar->sendText("Muster set successfully");
}

void Striker::setTarget(void)
{
	if (strikers.empty())
	{
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
		return NULL;
	}

	set<Unit*> workers;
	set<Unit*> attackers;
	set<Unit*> baseDefense;
	set<Unit*> benign;

	for (set<Unit*>::iterator i = shown.begin(); i != shown.end(); i++)
	{
		UnitType type = (*i)->getType();
		if (type.isWorker())
		{
			workers.insert(*i);
		}
		else if (type.canAttack())
		{
			if (type.isBuilding())
			{
				baseDefense.insert(*i);
			}
			else
			{
				attackers.insert(*i);
			}
		}
		else
		{
			benign.insert(*i);
		}
	}

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
	Broodwar->sendText("Checking all targets");
	set<Unit*> allUnits = Broodwar->getAllUnits();
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
	unitShown(unit);
}

void Striker::unitKilled(BWAPI::Unit* unit)
{
	if (isStriker(unit))
	{
		strikers.erase(unit);
	}
	else
	{
		target = NULL;
		shown.erase(unit);
	}
	setTarget();
}

void Striker::unitHidden(BWAPI::Unit* unit)
{
	Broodwar->sendText("Unit hidden");
	if (unit->getType().isFlyer())
	{
		return;
	}

	shown.erase(unit);
	hidden.push(unit->getPosition());

	if (unit == target)
	{
		target = NULL;
		setTarget();
	}
}

void Striker::unitShown(BWAPI::Unit* unit)
{	
	if (unit->getType().isFlyer())
	{
		return;
	}

	if (shown.find(unit) == shown.end() && unit->getPlayer()->isEnemy(Broodwar->self()))
	{
		shown.insert(unit);
	}

	setTarget();
}

void Striker::attackTarget(void)
{
	for (set<Unit*>::iterator i = strikers.begin(); i != strikers.end(); i++)
	{
		if (!(*i)->isAttacking() && !(*i)->isMoving())
		{
			if ((*i)->isUnderAttack() && !target->getType().canAttack())
			{
				setTarget();
			}
			(*i)->attack(target);
		}
	}
}

void Striker::attackPosition(void)
{
	set<Unit*> tileUnits = Broodwar->getUnitsOnTile(targetPosition.x(), targetPosition.y());
	if (Broodwar->isVisible(TilePosition(targetPosition)))
	{
		for (set<Unit*>::iterator i = tileUnits.begin(); i != tileUnits.end(); i++)
		{
			if (!(*i)->getPlayer()->isEnemy(Broodwar->self()))
				tileUnits.erase(i);
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

	if (false && !foundMuster)
	{
		Broodwar->sendText("No muster set");
		setMuster();
	}

	if (shown.empty() && hidden.empty())
	{
		checkAllTargets();
	}

	if (target == NULL)
	{
		attackPosition();
	} 
	else
	{
		attackTarget();
	}
}

// Private member functions
bool Striker::isVisible(Unit* unit)
{
	return shown.find(unit) != shown.end();
}