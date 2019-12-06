#include "LevelManager.h"
#include "Level.h"
#include "EarthLevel.h"
#include "FireLevel.h"
#include "WaterLevel.h"
#include "AirLevel.h"
#include "StrifeLevel.h"
#include "TestLevel.h"
#include "TDLevel.h"
#include "ProjectPickerLevel.h"



LevelManager::LevelManager(Systems& systems)
{
	systems._resMan.pushLevel(0);

	current = new TDLevel(systems);		//new ProjectPickerLevel(systems); boring to click through...
	current->init(systems);
	_levels.push_back(current);

	//needs to be able to load to gpu so needs device... also, resource manager doesn't belong in levels, keep it in systems

	//_levels.push_back(new EarthLevel(systems));
	//_levels.push_back(new FireLevel(systems));
	//_levels.push_back(new WaterLevel(systems));
	//_levels.push_back(new AirLevel(systems));
	//_levels.push_back(new Strife::StrifeLevel(systems));
}



LevelManager::~LevelManager() {}



void LevelManager::advanceLevel(Systems& systems)
{
	if (_levels.size() <= 1)
		return;

	delete _levels[0];
	_levels.erase(_levels.begin());
	_levels[0]->init(systems);
	current = _levels[0];
	return;
}



void LevelManager::update(Systems& systems, float dTime)
{
	sinceLastInput += dTime;

	if (sinceLastInput < .33f)
		return;

	if (systems._inputManager.isKeyDown((short)'L') || current->finished)
	{
		advanceLevel(systems);
		sinceLastInput = 0;
	}
}



void LevelManager::UpdateDrawCurrent(RenderContext& renderContext)
{
	current->update(renderContext);
	current->draw(renderContext);
}