#pragma once
#include <vector>

class Level;
class Systems;
class RenderContext;

class LevelManager
{
	std::vector<Level*> _levels;
	Level* current;

public:
	LevelManager(Systems& systems);
	~LevelManager();

	Level* advanceLevel(Systems& systems);
	void drawCurrentLevel(RenderContext& renderContext);
	void update(Systems& systems, float dTime);

	float sinceLastInput = 0.f;
};