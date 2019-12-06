#pragma once
#include <vector>

class Level;
class Systems;
class RenderContext;

class LevelManager
{
	float sinceLastInput = 0.f;
	std::vector<Level*> _levels;
	Level* current;

public:
	LevelManager(Systems& systems);
	~LevelManager();

	void advanceLevel(Systems& systems);
	void updateAndDrawCurrent(RenderContext& renderContext);
	void update(Systems& systems, float dTime);
	Level* getCurrentLevel() { return current; }
};