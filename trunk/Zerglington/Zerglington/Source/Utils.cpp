#include "Zerglington.h"
#include <BWAPI.h>
#include <queue>

using namespace BWAPI;

std::queue<std::string> morphQ; //Queue of what needs to be morphed. Must be a name of a unit type.

//Count the number of spawning pools
int Zerglington::spawningPoolCount(){
	int count = 0;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if(strcmp((*i)->getType().getName().c_str(), "Zerg Spawning Pool") == 0)
			count++;
	}
	return count;
}

//Given a unit i, return a pointer to the mineral patch it is closest to
Unit* Zerglington::findClosestMineral(Unit *i){
	Unit* closestMineral=NULL;
	for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
		if (closestMineral==NULL || (*i).getDistance(*m)<(*i).getDistance(closestMineral))
			closestMineral=*m;
	}
	return closestMineral;
}

//Sends a drone to mine at the nearest mineral patch
void Zerglington::sendToMine(BWAPI::Unit *unit){
	//Only give command if drone is not yet gathering minerals
	if(unit->isGatheringMinerals())
		return;
	else{
		Unit* closestMineral = findClosestMineral(unit);
		if (closestMineral!=NULL){
			unit->rightClick(closestMineral);
		}
	}
}

//Sends a drone to morph
void Zerglington::sendToMorph(BWAPI::Unit *unit){
	//Only give command if drone is not yet morphing
	if(unit->isMorphing())
		return;
	else if(morphQ.size() != 0){
		std::string type = morphQ.front();
		TilePosition p = getBuildLoc();
		Broodwar->sendText("Should build pool at (%d,%d)", p.x(), p.y());
		unit->rightClick(Position(p.x()*32,p.y()*32));
		//Broodwar->sendText("Gonna morph a %s at (%d,%d)", type, p.x(), p.y());
		//unit->rightClick(Position(p.x(),p.y()));
		//if(unit->morph(UnitTypes::getUnitType(type)) == true)
		//	morphQ.pop();
	}
}

//Determines the most needed job for a drone given the current situation
int Zerglington::mostNeededJob(){
	//Morph a spawning pool as soon as possible
	if(Broodwar->self()->minerals() >= UnitTypes::Zerg_Spawning_Pool.mineralPrice()
		&& spawningPoolCount() < 1){
			morphQ.push("Zerg Spawning Pool");
			return MORPH;
	}
	else return MINERALS;
}

//Finds a place to build (morph) a spawning pool
//Build such that we do not intersect the workers' path to the minerals.
//Take the location of the closest mineral patch to the hatchery, and build the pool
//directly on the opposite side of the hatchery.
TilePosition Zerglington::getBuildLoc(){
	//Get position of mineral patch closest to the hatchery
	TilePosition posHatchery;
	TilePosition posMinerals;
	TilePosition posPool;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if(strcmp((*i)->getType().getName().c_str(), "Zerg Hatchery") == 0){
			posHatchery = (*i)->getTilePosition();
			posMinerals = findClosestMineral(*i)->getTilePosition();
			//If we found the minerals in relation to the hatchery, now determine the position
			//On the far side of the hatchery from the minerals.
			//Then decrease the distance between where the hatchery is and where the pool will be
			//By a factor of j to get it as close as possible where we can still build there
			for(int j = 5; j > 0; j--){
				posPool.x() = posHatchery.x() + (posHatchery.x() - posMinerals.x())/j;
				posPool.y() = posHatchery.y() + (posHatchery.y() - posMinerals.y())/j;
				if(Broodwar->canBuildHere(NULL, posPool, UnitTypes::Zerg_Spawning_Pool)){
					Broodwar->sendText("Hatchery is at (%d,%d)", posHatchery.x(), posHatchery.y());
					return posPool;
				}
			}
		}
	}
	return BWAPI::TilePosition(-1,-1);
}
