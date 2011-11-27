#include "Zerglington.h"
#include <BWAPI.h>
using namespace BWAPI;

//Initialize the build order for a six-pool rush
void Zerglington::initBuildOrder(){
	morphQ.push(DRONE);
	morphQ.push(DRONE);
	morphQ.push(POOL);
	morphQ.push(DRONE);
	morphQ.push(LING);
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
	if(morphQ.front() == POOL && Broodwar->canMake(NULL, UnitTypes::Zerg_Spawning_Pool) && isMorphingSpawningPool == false){
		//Morph a spawning pool as soon as we can if we don't already have one
		isMorphingSpawningPool = true;	
		return MORPH;
	}
	//No drones need to morph, so get them to mine
	return MINERALS;
}

//Performs unit creation operations such as morphing drones and zerglings
void Zerglington::larvaMorphing(){
	//Broodwar->sendText("MorphQ size is %d", morphQ.size());
	//Check whether we have the resources to morph the next unit in the morph queue, and do it if possible
	if(morphQ.size() != 0){ //First see if there's anything left to morph
		if(morphQ.front() == DRONE && Broodwar->canMake(NULL, UnitTypes::Zerg_Drone)){
			//Case 1: Next in morph queue is a drone
			//Get the hatchery, then get the larva to morph into a drone
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isResourceDepot()){
					//Select one larvae and morph into drone.
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0){
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Drone);
						morphQ.pop(); //Remove this task from the queue
					}
				}
			}
		}
		else if(morphQ.front() == LING && Broodwar->canMake(NULL, UnitTypes::Zerg_Zergling)){
			//Case 2: Next in morph queue is a zergling
			//Get the hatchery, then get the larva to morph into a zergling
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isResourceDepot()){
					//Select one larvae and morph into zergling.
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0){
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Zergling);
						//morphQ.pop(); //Remove this task from the queue
					}
				}
			}
		}
		else if(morphQ.front() == LING && hasSpawningPool && Broodwar->canMake(NULL, UnitTypes::Zerg_Overlord)){
			//Case 3: Make an overlord so we can make more zerglings
			//Get the hatchery, then get the larva to morph into an overlord
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if ((*i)->getType().isResourceDepot()){
					//Select one larvae and morph into zergling.
					std::set<Unit*> myLarva=(*i)->getLarva();
					if (myLarva.size()>0){
						Unit* larva=*myLarva.begin();
						larva->morph(UnitTypes::Zerg_Overlord);
						//morphQ.pop(); //Remove this task from the queue
					}
				}
			}
		}
	}
	//}else{
		//The morph queue is empty, so continue building zerglings and overlords
		//Determine ratio of controlled units to overlords' max control
		/*int maxControl = 0;
		int controlledUnits = 0;
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
			if (strcmp((*i)->getType().getName().c_str(), "Zerg Overlord") == 0){
				maxControl+=8; //Get 8 control per overlord
			}else if(!(*i)->getType().isBuilding()){
				controlledUnits++;
			}
		}
		if(controlledUnits < maxControl && Broodwar->canMake(NULL, UnitTypes::Zerg_Zergling)){
			//Morph a zergling if possible
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if (strcmp((*i)->getType().getName().c_str(), "Zerg Larva") == 0){
					(*i)->morph(UnitTypes::Zerg_Zergling);
					break;
				}
			}
		}else if(controlledUnits >= maxControl && Broodwar->canMake(NULL, UnitTypes::Zerg_Overlord)){
			//Morph an overlord if we need to increase control
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
				if (strcmp((*i)->getType().getName().c_str(), "Zerg Larva") == 0){
					(*i)->morph(UnitTypes::Zerg_Overlord);
					break;
				}
			}
		}*/
	//}
}

//Finds a place to build a spawning pool
//Build such that we do not intersect the workers' path to the minerals.
//Take the location of the closest mineral patch to the hatchery, and build the pool
//directly on the opposite side of the hatchery from that.
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
			//on the far side of the hatchery from the minerals.
			//Then, decrease the distance between where the hatchery is and where the pool will be
			//by a factor of j to get it as close as possible where we can still build there
			for(int j = 5; j > 0; j--){
				posPool.x() = posHatchery.x() + (posHatchery.x() - posMinerals.x())/j;
				posPool.y() = posHatchery.y() + (posHatchery.y() - posMinerals.y())/j;
				if(Broodwar->canBuildHere(NULL, posPool, UnitTypes::Zerg_Spawning_Pool)){
					return posPool;
				}
			}
		}
	}
	return BWAPI::TilePosition(-1,-1);
}
