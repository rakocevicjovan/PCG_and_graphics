#include "LevelManager.h"
#include "Level.h"
#include "StrifeLevel.h"
#include "TestLevel.h"
#include "TDLevel.h"
#include "ProjectPickerLevel.h"
#include "AssimpLoader.h"



LevelManager::LevelManager(Engine& systems)
{	
	//current = new ProjectPickerLevel(systems);	//boring to click through...
	
	//current = new TDLevel(systems);		

	current = new AssimpLoader(systems);

	current->init(systems);
	_levels.push_back(current);
}



LevelManager::~LevelManager() {}



void LevelManager::advanceLevel(Engine& systems)
{
	if (_levels.size() <= 1)
		return;

	delete _levels[0];
	_levels.erase(_levels.begin());
	_levels[0]->init(systems);
	current = _levels[0];
}



void LevelManager::handleInput(Engine& systems, float dTime)
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



void LevelManager::updateAndDrawCurrent(RenderContext& renderContext)
{
	current->update(renderContext);
	current->draw(renderContext);
}