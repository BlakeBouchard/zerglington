#include "Zerglington.h"
#include <BWAPI.h>

using namespace BWAPI;

/** Given a unit i, return a pointer to the mineral patch it is closest to*/ 
Unit* Zerglington::findClosestMineral(Unit *i){
	Unit* closestMineral=NULL;
	for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
		if (closestMineral==NULL || (*i).getDistance(*m)<(*i).getDistance(closestMineral))
			closestMineral=*m;
	}
	return closestMineral;
}

//Sets all drones that are currently not mining to mine at the nearest mineral patch
void Zerglington::dronesMine(){
	//Iterate through all units
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		//Send all idle workers to the nearest mineral patch
		if ((*i)->getType().isWorker() 
			&& !(*i)->isGatheringMinerals() && !(*i)->isGatheringGas()){
			Unit* closestMineral = findClosestMineral(*i);
			if (closestMineral!=NULL){
				(*i)->rightClick(closestMineral);
			}
		}
	}
}