#include "Worker.h"

Worker::Worker(int j, BWAPI::Unit* u){
	job = j;
	unit = u;
	id = u->getID();
}

Worker::~Worker(void)
{
}

bool Worker::isSame(int id_){
	if(id == id_)
		return true;
	return false;
}
