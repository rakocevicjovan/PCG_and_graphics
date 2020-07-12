#pragma once
#include <vector>

class Level;
class Engine;
class RenderContext;

class LevelManager
{
	float sinceLastInput = 0.f;
	std::vector<Level*> _levels;
	Level* _current;

public:
	LevelManager(Engine& systems);
	~LevelManager();

	void advanceLevel(Engine& systems);
	void updateAndDrawCurrent(RenderContext& renderContext);
	void handleInput(Engine& systems, float dTime);
	Level* getCurrentLevel() { return _current; }
};