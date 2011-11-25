#include "Zerglington.h"
#include <BWAPI.h>
using namespace BWAPI;

//Initialize the build order for a six-pool rush
void Zerglington::initBuildOrder(){
	morphQ.push(DRONE);
	morphQ.push(DRONE);
	morphQ.push(POOL);
	morphQ.push(DRONE);
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

//Sends a drone to morph into a spawning pool
void Zerglington::sendToMorph(BWAPI::Unit *unit){
	unit->build(posBuild, UnitTypes::Zerg_Spawning_Pool);
	if(unit->isMorphing()){
		morphQ.pop(); //Remove this task from the queue
		workers.erase(unit->getID()); //Remove this worker from the queue so we don't mess with him
	}
}

//Determines the most needed job for a drone given the current situation
int Zerglington::mostNeededJob(){
	//Check whether we have the resources to morph the next unit in the morph queue, and do it if possible
	if(morphQ.size() != 0){ //First see if there's anything left to morph
		if(morphQ.front() == DRONE && Broodwar->canMake(NULL, UnitTypes::Zerg_Drone)){
			//Get the hatchery, then get the larva
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isResourceDepot()){
					//Select a larvae and morph into drone.
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0){
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Drone);
						morphQ.pop(); //Remove this task from the queue
					}
				}
			}
		}
		else if(morphQ.front() == POOL && Broodwar->canMake(NULL, UnitTypes::Zerg_Spawning_Pool) && isMorphingSpawningPool == false){
			isMorphingSpawningPool = true;	
			return MORPH;
		}
	}
	
	//No drones need to morph, so get them to mine
	return MINERALS;
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
