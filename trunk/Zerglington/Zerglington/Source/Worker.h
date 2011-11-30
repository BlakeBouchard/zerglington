#pragma once
#include <BWAPI.h>
#include <queue>
#include <map>

enum job{IDLE, MINERALS, GAS, MORPH};
enum morph{DRONE, POOL, LORD, LING};

//Class Worker keeps track of useful aspects of a drone
//by setting the job it should be doing
class Worker{
public:
	Worker(int j, BWAPI::Unit* u);
	~Worker(void);
	
	//Access functions
	void setJob(int j){ job = j;}
	int getJob(){ return job; }
	BWAPI::Unit* getUnit(){return unit; }
	
	bool isSame(int id_);

private:
	int job;
	int id;
	BWAPI::Unit* unit;
};

//Class WorkerManager keeps track of all allied drones in the game.
//Sets their jobs, sends them to carry out the jobs, as well as
//morphing larva into the most needed unit types
class WorkerManager{
public:
	WorkerManager(){
		initBuildOrder(); //Initialize the build order
		posBuild = getBuildLoc(); //Determine where the spawning pool should be placed
		hasSpawningPool = false;
		isMorphingSpawningPool = false;
	}
	~WorkerManager(){ workers.clear(); }

	void addWorker(BWAPI::Unit* w);
	void removeWorker(BWAPI::Unit* w);

	//Determines the most needed job of a worker given the current conditions
	int mostNeededJob();
	//Performs unit creation operations such as morphing drones and zerglings
	void larvaMorphing();

	//Carry out the jobs assigned to each worker
	void manageWorkers();
	void sendWorkerMine(Worker* w);
	void sendWorkerMorph(Worker* w);

	//Utilities for managing larva and workers:

	//Initialize the build order for a six-pool rush
	void initBuildOrder();
	//Finds a place to build (morph) a spawning pool
	BWAPI::TilePosition getBuildLoc();
	//Finds the closest mineral patch to a given unit
	BWAPI::Unit* findClosestMineral(BWAPI::Unit* unit);
	//Checks whether a building spawning pool is finished
	void checkSpawningPool();
	//Finds one larva and returns a pointer to it
	BWAPI::Unit* getOneLarva();

	std::map<int, Worker*> workers; //Map organizing all drone units, keyed by their ID
	std::queue<int> morphQ; //Queue of what needs to be morphed. Must correspond to enum morph

	BWAPI::TilePosition posBuild; //The tile position where we will build our spawning pool
	bool hasSpawningPool;
	bool isMorphingSpawningPool;
};
