#pragma once
#include "TDController.h"
#include "TDGUI.h"
#include "Economy.h"
#include "IndustrialBuilding.h"
#include "Tower.h"
#include "Enemy.h"
#include "Building.h"

class TDGame
{
public:

	TDController _tdController;
	TDGUI _tdgui;

	Building* _templateBuilding = nullptr;	//not owning
	Building* _selectedBuilding = nullptr;	//not owning
	bool _inBuildingMode = false;


	std::vector<Building*> _buildable;
	//@TODO replace all three with SlotVector

	std::list<Tower> _towers;
	std::list<IndustrialBuilding> _industry;
	std::list<Building*> _structures;
	//the master list type-slices but it doesn't matter, it's just there to use it AS a base class - example, rendering


	void addBuildables();
	void fixBuildable(Building* b);
	void build();
	void demolish();
	void rayPickTerrain(const Camera* cam);
	Building* rayPickBuildings(const Camera* cam);
	void handleInput(const Camera* cam);	//involves ray picking, so we need this
	void steerEnemies(float dTime);
	float resolveAttack(const Attack& att, const Armour& arm);
};