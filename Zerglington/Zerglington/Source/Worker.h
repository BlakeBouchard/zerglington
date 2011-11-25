#pragma once
#include <BWAPI.h>

enum job{IDLE, MINERALS, GAS, MORPH};

class Worker{
public:
	Worker(int j, BWAPI::Unit* u);
	~Worker(void);
	void setJob(int j){ job = j;}
	int getJob(){ return job; }
	BWAPI::Unit* getUnit(){return unit; }
	
	bool isSame(int id_);

private:
	int job;
	int id;
	BWAPI::Unit* unit;
};
