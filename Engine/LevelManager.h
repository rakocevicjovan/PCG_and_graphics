#pragma once

class Level;
class Engine;
struct RenderContext;

// @TODO delet this
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