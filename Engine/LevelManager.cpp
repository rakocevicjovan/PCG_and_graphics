#include "LevelManager.h"
#include "Level.h"

LevelManager::LevelManager(Systems& systems) //: sys(systems)
{
	current = new EarthLevel(systems);
	current->init(systems);
	_levels.push_back(current);
	_levels.push_back(new FireLevel(systems));
	_levels.push_back(new WaterLevel(systems));
	_levels.push_back(new AirLevel(systems));
}



LevelManager::~LevelManager()
{
}



Level* LevelManager::advanceLevel(Systems& systems)
{
	if (_levels.size() <= 1)
		return _levels[0];

	_levels.front()->demolish();
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

	if (systems._inputManager.IsKeyDown((short)'L'))
	{
		advanceLevel(systems);
		sinceLastInput = 0;
	}
}



void LevelManager::drawCurrentLevel(RenderContext& renderContext)
{
	current->draw(renderContext);
}