#include "Zerglington.h"
#include <BWAPI.h>

using namespace BWAPI;

Unit* ExampleAIModule::findClosestMineral(Unit *i){
	Unit* closestMineral=NULL;
	for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++){
		if (closestMineral==NULL || (*i).getDistance(*m)<(*i).getDistance(closestMineral))
			closestMineral=*m;
	}
	return closestMineral;
}