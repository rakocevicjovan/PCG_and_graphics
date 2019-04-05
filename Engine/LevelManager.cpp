#include "LevelManager.h"
#include "Level.h"


LevelManager::LevelManager(Systems& systems) //: sys(systems)
{
	current = new EarthLevel(systems);
	current->init(systems);
	_levels.push_back(current);
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
	return _levels[0];
}

void LevelManager::drawCurrentLevel(RenderContext& renderContext)
{
	current->draw(renderContext);

}
