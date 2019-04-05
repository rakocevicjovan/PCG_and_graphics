#include "LevelManager.h"



LevelManager::LevelManager(Systems& systems) : sys(systems)
{
	EarthLevel earthLevel(sys);

	_levels.push_back(new EarthLevel(sys));
}



LevelManager::~LevelManager()
{
}



Level* LevelManager::advanceLevel()
{
	if (_levels.size() <= 1)
		return _levels[0];

	_levels.front()->demolish();
	_levels.erase(_levels.begin());
	_levels[0]->init(sys);
	return _levels[0];
}