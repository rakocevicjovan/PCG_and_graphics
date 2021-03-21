#include "LevelManager.h"
#include "Level.h"
#include "StrifeLevel.h"
#include "TestLevel.h"
#include "TDLevel.h"
#include "ProjectPickerLevel.h"
#include "AssimpLoader.h"
#include "RenderingTestLevel.h"



LevelManager::LevelManager(Engine& systems)
{	
	//_current = new ProjectPickerLevel(systems);	//boring to click through...

	//_current = new AssimpLoader(systems);

	//ig_current = new TDLevel(systems);

	_current = new RenderingTestLevel(systems);

	_current->init(systems);
	_levels.push_back(_current);
}



LevelManager::~LevelManager() {}



void LevelManager::advanceLevel(Engine& systems)
{
	if (_levels.size() <= 1)
		return;

	delete _levels[0];
	_levels.erase(_levels.begin());
	_levels[0]->init(systems);
	_current = _levels[0];
}



void LevelManager::handleInput(Engine& systems, float dTime)
{
	sinceLastInput += dTime;

	if (sinceLastInput < .33f)
		return;

	if (systems._inputManager.isKeyDown((short)'L') || _current->finished)
	{
		advanceLevel(systems);
		sinceLastInput = 0;
	}
}



void LevelManager::updateAndDrawCurrent(RenderContext& renderContext)
{
	_current->update(renderContext);
	_current->draw(renderContext);
}