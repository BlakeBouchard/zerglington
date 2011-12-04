#include "Worker.h"
using namespace BWAPI;

/*--------Worker-------------*/

Worker::Worker(int j, BWAPI::Unit* u){
	job = j;
	unit = u;
	id = u->getID();
}

Worker::~Worker(void){}

bool Worker::isSame(int id_){
	if(id == id_)
		return true;
	return false;
}

/*--------Worker Manager-------------*/

WorkerManager::WorkerManager(){
	initBuildOrder(); //Initialize the build order
	hasSpawningPool = false;
	isMorphingSpawningPool = false;
	hasHatchery2 = false;
	isMorphingHatchery2 = false;
}

WorkerManager::~WorkerManager(){}

//Initializes build order for a six pool rush
void WorkerManager::initBuildOrder(){
	morphQ.push(DRONE);
	morphQ.push(DRONE);
	morphQ.push(POOL);
	morphQ.push(DRONE);
	morphQ.push(LORD);
	morphQ.push(LING);
}

//Add a worker to the workers q, keyed by its id
void WorkerManager::addWorker(Unit* w){
	workers.insert(std::pair<int, Worker*>(w->getID(), new Worker(IDLE, w)));
}

//Remove a worker from the workers q
void WorkerManager::removeWorker(Unit* u){
	workers.erase(u->getID());
}

//Determines the most needed job for a drone given the current situation
int WorkerManager::mostNeededJob(){
	if(morphQ.front() == POOL && Broodwar->canMake(NULL, UnitTypes::Zerg_Spawning_Pool) && !isMorphingSpawningPool){
		//Morph a spawning pool as soon as we can if we don't already have one
		isMorphingSpawningPool = true;	
		return MORPHPOOL;
	}else if(Broodwar->canMake(NULL, UnitTypes::Zerg_Hatchery) && hasSpawningPool && !isMorphingHatchery2){
		//Morph a second hatchery after we have a spawning pool
		isMorphingHatchery2 = true;
		return MORPHHATCHERY;
	}
	//No drones need to morph, so get them to mine
	return MINERALS;
}

//Carry out workers' actions based on their assigned jobs
void WorkerManager::manageWorkers(){
	//Mange Workers
	for(std::map<int, Worker*>::const_iterator i = workers.begin(); i != workers.end(); i++){
		//First determine the role the unit should have (morphing takes priority, don't change that job)
		if((*i).second->getJob() != MORPHPOOL && (*i).second->getJob() != MORPHHATCHERY)
			(*i).second->setJob(mostNeededJob());
		
		//Perform the unit's job
		if((*i).second->getJob() == MINERALS){			//Send to mineral patch
			sendWorkerMine((*i).second);
		}
		else if((*i).second->getJob() == MORPHPOOL){	//Send to morph into spawning pool
			sendWorkerMorphPool((*i).second);
		}else if((*i).second->getJob() == MORPHHATCHERY){//Send to morph into hatchery
			sendWorkerMorphHatchery((*i).second);
		}
	}
}

//Send a drone to mine at the nearest mineral patch
void WorkerManager::sendWorkerMine(Worker* worker){
	//Only give command if drone is not yet gathering minerals
	if(worker->getUnit()->isGatheringMinerals())
		return;
	else{
		Unit* closestMineral = findClosestMineral(worker->getUnit());
		if (closestMineral!=NULL){
			worker->getUnit()->rightClick(closestMineral);
		}
	}
}

//Sends a drone to morph into a spawning pool
void WorkerManager::sendWorkerMorphPool(Worker* worker){
	TilePosition pos = getBuildLocPool();
	bool temp = worker->getUnit()->build(pos, UnitTypes::Zerg_Spawning_Pool);
	Broodwar->sendText("%s tries to be spawning pool at %d %d, result is %d",
		worker->getUnit()->getType().getName().c_str(),
		pos.x(), pos.y(), temp);
	if(worker->getUnit()->isMorphing()){
		morphQ.pop(); //Remove this task from the queue
		workers.erase(worker->getUnit()->getID()); //Remove this worker from the queue so we don't mess with him
	}
}

//Sends a drone to morph into a hatchery
void WorkerManager::sendWorkerMorphHatchery(Worker* worker){
	TilePosition pos = getBuildLocHatchery();
	bool temp = worker->getUnit()->build(pos, UnitTypes::Zerg_Hatchery);
	Broodwar->sendText("%s tries to be a hatchery at %d %d, result is %d",
		worker->getUnit()->getType().getName().c_str(),
		pos.x(), pos.y(), temp);
	if(worker->getUnit()->isMorphing()){
		workers.erase(worker->getUnit()->getID()); //Remove this worker from the queue so we don't mess with him
	}
}

//Return a pointer to the mineral patch closest to the unit
Unit* WorkerManager::findClosestMineral(Unit* unit){
	Unit* closestMineral=NULL;
	for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
		if (closestMineral==NULL || unit->getDistance(*m)< unit->getDistance(closestMineral))
			closestMineral=*m;
	}
	return closestMineral;
}


//Checks whether a building spawning pool is finished
void WorkerManager::checkSpawningPool(){
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		//Find a spawning pool -> if no longer morphing, set flag to true
		if (strcmp((*i)->getType().getName().c_str(), "Zerg Spawning Pool") == 0
			&& (*i)->isCompleted()){
				hasSpawningPool = true;
				return;
		}
	}
}

//Checks whether a building hatchery is finished
void WorkerManager::checkHatchery2(){
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		//Find all hatchery -> if any one is not complete, return without setting flag to true
		if (strcmp((*i)->getType().getName().c_str(), "Zerg Hatchery") == 0
			&& !(*i)->isCompleted()){
				return;
		}
	}
	hasHatchery2 = true;
}

//Determines whether we need to morph an overlord based on the current control (supply) ratio
bool WorkerManager::needOverlord(){
	double ratio = Broodwar->self()->supplyUsed() / Broodwar->self()->supplyTotal();
	if( ratio >= 0.8)
		return true;
	return false;
}

//Performs unit creation operations such as morphing drones and zerglings
void WorkerManager::larvaMorphing(){
	if(isMorphingSpawningPool && !hasSpawningPool)  //Check whether our pool is finished
		checkSpawningPool();
	if(isMorphingHatchery2 && !hasHatchery2)		//Check whether second hatchery is finished
		checkHatchery2();

	//Check whether we have the resources to morph the next unit in the morph queue, and do it if possible
	if(morphQ.size() != 0){ //First see if there's anything left to morph
		if(morphQ.front() == DRONE && Broodwar->canMake(NULL, UnitTypes::Zerg_Drone)){
			//Case 1: Next in morph queue is a drone
			Unit* larva = getOneLarva();
			larva->morph(UnitTypes::Zerg_Drone);
			morphQ.pop(); //Remove this task from the queue
		}
		else if(morphQ.front() == LORD && Broodwar->canMake(NULL, UnitTypes::Zerg_Overlord)){
			//Case 2: Next in morph queue is an overlord
			//Get the hatchery, then get the larva to morph into an overlord
			Unit* larva = getOneLarva();
			larva->morph(UnitTypes::Zerg_Overlord);
			morphQ.pop(); //Remove this task from the queue
		}
		else if(morphQ.front() == LING && Broodwar->canMake(NULL, UnitTypes::Zerg_Zergling)){
			//Case 3a: Next in morph queue is a zergling. This means that it's time to pump out zerglings nonstop
			Unit* larva = getOneLarva();
			larva->morph(UnitTypes::Zerg_Zergling);
			//We no longer pop the task from the queue because we want to keep making zerglings
		}else if(morphQ.front() == LING && hasSpawningPool && Broodwar->canMake(NULL, UnitTypes::Zerg_Overlord)){
			//Case 3b: We want zerglings, we have a spawning pool, but we need more overlords to increase control
			//Get the hatchery, then get the larva to morph into an overlord
			Unit* larva = getOneLarva();
			larva->morph(UnitTypes::Zerg_Overlord);
			//We do not pop a task from the queue because we want to continue making zerglings or overlords when necessary
		}
	}
}

//Finds one larva and returns a pointer to it
Unit* WorkerManager::getOneLarva(){
	//Get the hatchery, then get the larva and return it
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if ((*i)->getType().isResourceDepot()){
			//Select one the first larva of the set
			std::set<Unit*> myLarva=(*i)->getLarva();
			if (myLarva.size()>0){
				Unit* larva=*myLarva.begin();
				return larva;
			}
		}
	}
	return NULL;
}

//Finds a place to build a spawning pool
//Build such that we do not intersect the workers' path to the minerals.
//Take the location of the closest mineral patch to the hatchery, and build the pool
//directly on the opposite side of the hatchery from that.
TilePosition WorkerManager::getBuildLocPool(){
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

//Finds a place to build a hatchery
//First, get the location of the spawning pool (we will only build hatchery #2 after spawning pool
//Then, test building locations 1 tile to each side of the pool
//Keep increasing distance from spawning pool by 1 tile, check in a spiral pattern until we can build
//This helps ensure that we will not build in the workers' path, since the original hatchery will block
//that build location and we should find a better one sooner.
TilePosition WorkerManager::getBuildLocHatchery(){
	TilePosition posPool;
	TilePosition posHatchery;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++){
		if(strcmp((*i)->getType().getName().c_str(), "Zerg Spawning Pool") == 0){
			posPool = (*i)->getTilePosition();
			//Now look directly above, to each side, and below the pool, increasing distance
			//from the pool gradually until we find a place to build.
			for(int j = 1; j <= 6; j++){
				//North of pool
				posHatchery.x() = posPool.x();
				posHatchery.y() = posPool.y() - j;
				if(Broodwar->canBuildHere(NULL, posHatchery, UnitTypes::Zerg_Hatchery)){
					return posHatchery;
				}
				
				//East of pool
				posHatchery.x() = posPool.x() + j;
				posHatchery.y() = posPool.y();
				if(Broodwar->canBuildHere(NULL, posHatchery, UnitTypes::Zerg_Hatchery)){
					return posHatchery;
				}

				//South of pool
				posHatchery.x() = posPool.x();
				posHatchery.y() = posPool.y() + j;
				if(Broodwar->canBuildHere(NULL, posHatchery, UnitTypes::Zerg_Hatchery)){
					return posHatchery;
				}

				//West of pool
				posHatchery.x() = posPool.x() -j;
				posHatchery.y() = posPool.y();
				if(Broodwar->canBuildHere(NULL, posHatchery, UnitTypes::Zerg_Hatchery)){
					return posHatchery;
				}
			}
		}
	}
	return BWAPI::TilePosition(-1,-1);
}
