#include "Scouter.h"

using namespace BWAPI;
using namespace std;

vector<Unit*> overlords;
vector<Unit*> zerglings;
set<BWTA::BaseLocation*> startLocations;

Scouter::Scouter()
{
	startLocations = BWTA::getStartLocations();
}

Scouter::~Scouter(void)
{
}

void Scouter::addOverlord(Unit* overlord)
{
	overlords.push_back(overlord);
}

void Scouter::addZergling(Unit* zergling)
{
	zerglings.push_back(zergling);
}

void Scouter::foundUnit(Unit* unit)
{

}

BWTA::Region* Scouter::updateScouts(void)
{
	return NULL;
}