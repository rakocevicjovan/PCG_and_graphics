#include "LevelManager.h"
#include "Level.h"
#include "EarthLevel.h"
#include "FireLevel.h"
#include "WaterLevel.h"
#include "AirLevel.h"



LevelManager::LevelManager(Systems& systems)
{
	current = new WaterLevel(systems);
	current->init(systems);
	_levels.push_back(current);
	_levels.push_back(new FireLevel(systems));
	_levels.push_back(new WaterLevel(systems));
	_levels.push_back(new AirLevel(systems));
}



LevelManager::~LevelManager() {}



Level* LevelManager::advanceLevel(Systems& systems)
{
	if (_levels.size() <= 1)
		return _levels[0];

	_levels.front()->demolish();
	delete _levels.front();
	_levels.erase(_levels.begin());
	_levels[0]->init(systems);
	current = _levels[0];
	return _levels[0];
}



void LevelManager::update(Systems& systems, float dTime)
{
	sinceLastInput += dTime;

	if (sinceLastInput < .33f)
		return;

	if (systems._inputManager.IsKeyDown((short)'L') || current->finished)
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