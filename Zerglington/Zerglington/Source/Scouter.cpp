#include "Scouter.h"

using namespace BWAPI;
using namespace std;

vector<Unit*> scouts;
set<BWTA::BaseLocation*> startLocations;
set<BWTA::BaseLocation*> unscouted;
map<BWTA::BaseLocation*, Unit* > enroute;
BWTA::BaseLocation* enemyBase;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

Scouter::Scouter(void)
{
	analyzed = false;
	analysis_just_finished = false;
	enemyBase = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::initialize(void)
{
	BWTA::readMap();
	Broodwar->printf("Analyzing map... this may take a minute");
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
}

void Scouter::addOverlord(Unit* overlord)
{
	scouts.push_back(overlord);
	Broodwar->sendText("Added Overlord to scouts");
}

void Scouter::addZergling(Unit* zergling)
{
	scouts.push_back(zergling);
	Broodwar->sendText("Added Zergling to scouts");
}

BWTA::BaseLocation* findNearestUnscouted(Unit* unit)
{
	set<BWTA::BaseLocation*>::iterator i = unscouted.begin();
	BWTA::BaseLocation* closest = (*i);
	Position unitPosition = unit->getPosition();
	
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i)->getPosition().getDistance(unitPosition) < closest->getPosition().getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

void Scouter::foundBase(BWTA::BaseLocation* base)
{
	enemyBase = base;
	Broodwar->sendText("Found enemy base");
}

bool Scouter::foundEnemyBase()
{
	return enemyBase != NULL;
}

void Scouter::foundUnit(Unit* unit)
{
	BWTA::Region* enemyRegion = BWTA::getRegion(unit->getTilePosition());

	// Determine if enemy unit's location is the same region as a possible enemy starting location
	for (set<BWTA::BaseLocation*>::iterator i = startLocations.begin(); i != startLocations.end(); i++)
	{
		if ((*i)->getRegion() == enemyRegion)
		{
			foundBase(*i);
			break;
		}
	}
}

BWTA::Region* Scouter::getEnemyBase(void)
{
	if (foundEnemyBase())
		return enemyBase->getRegion();
	else
		return NULL;
}

void Scouter::updateScouts(void)
{
	if (analysis_just_finished)
	{
		startLocations = BWTA::getStartLocations();
		unscouted = startLocations;
		Broodwar->printf("Found %d start locations", startLocations.size());
		analysis_just_finished = false;
	}

	if (analyzed)
	{
		if (!unscouted.empty())
		{
			for (vector<Unit*>::iterator i = scouts.begin(); i != scouts.end(); i++)
			{
				if ((*i)->isIdle())
				{
					BWTA::BaseLocation* nearest = findNearestUnscouted((*i));
					(*i)->move(nearest->getPosition());
					unscouted.erase(nearest);
					enroute.insert(pair<BWTA::BaseLocation*, Unit*>(nearest, (*i)));
					Broodwar->sendText("Scout moving to position");
				}
			}
		}
		if (!enroute.empty())
		{
			for (map<BWTA::BaseLocation*, Unit*>::iterator i = enroute.begin(); i != enroute.end(); i++)
			{
				if (i->first->getPosition() == i->second->getPosition())
				{
					enroute.erase(i);
				}
			}
		}
		if (!foundEnemyBase() && enroute.empty() && unscouted.empty())
		{
			Broodwar->sendText("WARNING: No more BaseLocations to scout, no bases found");
		}
	}
	else
	{
		for (vector<Unit*>::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			if ((*i)->isIdle())
			{
				
			}
		}
	}
}

DWORD WINAPI AnalyzeThread(){
	BWTA::analyze();

	//self start location only available if the map has base locations
	if (BWTA::getStartLocation(Broodwar->self())!=NULL){
		home       = BWTA::getStartLocation(Broodwar->self())->getRegion();
	}

	analyzed   = true;
	analysis_just_finished = true;
	Broodwar->sendText("Terrain analysis complete");
	return 0;
}


void Scouter::drawTerrainData(){
	//we will iterate through all the base locations, and draw their outlines.
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++){
		TilePosition p=(*i)->getTilePosition();
		Position c=(*i)->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

		//draw a circle at each mineral patch
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++){
			Position q=(*j)->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}

		//draw the outlines of vespene geysers
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++){
			TilePosition q=(*j)->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++){
		BWTA::Polygon p=(*r)->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++){
		for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++){
			Position point1=(*c)->getSides().first;
			Position point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
		}
	}
}