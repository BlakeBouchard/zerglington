#pragma once
#include <BWAPI.h>
#include "Worker.h"

//#include <BWTA.h>
#include <windows.h>
#include <queue>
#include <map>

enum morph{DRONE, POOL};

//extern bool analyzed;
//extern bool analysis_just_finished;
//extern BWTA::Region* home;
//extern BWTA::Region* enemy_base;
//DWORD WINAPI AnalyzeThread();

class Zerglington : public BWAPI::AIModule
{
public:
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player* player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player* player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit* unit);
	virtual void onUnitEvade(BWAPI::Unit* unit);
	virtual void onUnitShow(BWAPI::Unit* unit);
	virtual void onUnitHide(BWAPI::Unit* unit);
	virtual void onUnitCreate(BWAPI::Unit* unit);
	virtual void onUnitDestroy(BWAPI::Unit* unit);
	virtual void onUnitMorph(BWAPI::Unit* unit);
	virtual void onUnitRenegade(BWAPI::Unit* unit);
	virtual void onSaveGame(std::string gameName);
	void drawStats(); //not part of BWAPI::AIModule
	void drawBullets();
	void drawVisibilityData();
	//void drawTerrainData();
	void showPlayers();
	void showForces();
	bool show_bullets;
	bool show_visibility_data;

	std::queue<int> morphQ; //Queue of what needs to be morphed. Must correspond to enum morph
	std::map<int, Worker*> workers; //Map organizing all drone units, keyed by their ID
	BWAPI::TilePosition posBuild; //The tile position where we will build our spawning pool
	bool hasSpawningPool;
	bool isMorphingSpawningPool;
	int droneCount;

	/* Functions implemented in Utils.cpp */

	//Initialize the build order for a six-pool rush
	void initBuildOrder();

	//Finds the closest mineral patch to a given unit
	BWAPI::Unit* findClosestMineral(BWAPI::Unit* unit);

	//Sends a unit to mine at the nearest mineral patch
	void sendToMine(BWAPI::Unit* unit);

	//Sends a unit to morph
	void sendToMorph(BWAPI::Unit* unit);

	//Determines the most needed job of a worker given the current conditions
	int mostNeededJob();

	//Finds a place to build (morph) a spawning pool
	BWAPI::TilePosition getBuildLoc();
};
