#include "Scouter.h"

using namespace BWAPI;
using namespace std;

bool foundEnemyBase;
bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

Scouter::Scouter(void)
{
	analyzed = false;
	analysis_just_finished = false;
	foundEnemyBase = false;

	firstOverlord = NULL;
}

Scouter::~Scouter(void)
{
}

void Scouter::initialize(void)
{
	homeBase = Broodwar->self()->getStartLocation();
	startLocations = Broodwar->getStartLocations();
	unscouted = startLocations;
	unscouted.erase(homeBase);
	if (unscouted.size() == 1)
	{
		foundBase(*unscouted.begin());
	}
	BWTA::readMap();
	Broodwar->printf("Analyzing map... this may take a minute");
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
}

void Scouter::addOverlord(Unit* overlord)
{
	if (unscouted.empty())
		return;
	if (firstOverlord == NULL)
		firstOverlord = overlord;
	TilePosition destination = findNearestUnscouted(overlord);
	scouts.insert(ScoutPair(overlord, destination));
	unscouted.erase(destination);
	Broodwar->sendText("Added Overlord to scouts");
}

void Scouter::addZergling(Unit* zergling)
{
	if (unscouted.empty())
		return;
	TilePosition destination = findFurthestUnscouted(zergling);
	scouts.insert(ScoutPair(zergling, destination));
	unscouted.erase(destination);
	Broodwar->sendText("Added Zergling to scouts");
}

void Scouter::dumpZerglings(void)
{
	for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
	{
		if (i->first->getType().getID() == UnitTypes::Zerg_Zergling)
		{
			scouts.erase(i);
		}
	}
}

TilePosition Scouter::findFurthestUnscouted(Unit* unit)
{
	if (unscouted.empty())
		return homeBase;
	TileSet::iterator i = unscouted.begin();
	TilePosition furthest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i).getDistance(unitPosition) > furthest.getDistance(unitPosition))
		{
			furthest = (*i);
		}
	}

	return furthest;
}

TilePosition Scouter::findNearestUnscouted(Unit* unit)
{
	if (unscouted.empty())
		return homeBase;
	TileSet::iterator i = unscouted.begin();
	TilePosition closest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (++i; i != unscouted.end(); i++)
	{
		if ((*i).getDistance(unitPosition) < closest.getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

TilePosition Scouter::findNearestStart(Unit* unit)
{
	TileSet::iterator i = startLocations.begin();
	TilePosition closest = (*i);
	TilePosition unitPosition = unit->getTilePosition();
	
	for (; i != startLocations.end(); i++)
	{
		if ((*i).getDistance(unitPosition) < closest.getDistance(unitPosition))
		{
			closest = (*i);
		}
	}

	return closest;
}

void Scouter::foundBase(TilePosition basePosition)
{
	enemyBase = basePosition;
	foundEnemyBase = true;
	Broodwar->sendText("Found enemy base");
	dumpZerglings();
}

void Scouter::foundUnit(Unit* unit)
{
	UnitType unitType = unit->getType();
	if (!foundEnemyBase && unitType.isBuilding() && !unitType.isNeutral())
	{
		foundBase(unit->getTilePosition());
	}
}

TilePosition Scouter::getEnemyBase(void)
{
	return enemyBase;
}

void Scouter::updateScouts(void)
{
	if (!foundEnemyBase)
	{
		for (ScoutMap::iterator i = scouts.begin(); i != scouts.end(); i++)
		{
			Unit* scout = i->first;
			Position destination = Position(i->second);
			if (scout->isUnderAttack())
			{
				if (scout->getType().isFlyer())
				{
					TilePosition nearest = findNearestUnscouted(scout);
					unscouted.insert(i->second);
					scout->move(Position(nearest));
					i->second = nearest;
				}
				else
				{
					TilePosition furthest = findFurthestUnscouted(scout);
					unscouted.insert(i->second);
					scout->move(Position(furthest));
					i->second = furthest;
				}
			}
			else if (Broodwar->isVisible(i->second))
			{
				i->second = findNearestUnscouted(scout);
				scout->move(Position(i->second));
			}
			else
			{
				scout->move(Position(i->second));
			}
		}
	}
	else
	{
		
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