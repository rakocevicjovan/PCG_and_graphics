#pragma once
#include "Level.h"


class LevelManager
{
	Systems& sys;
	std::vector<Level*> _levels;

public:
	LevelManager(Systems& systems);
	~LevelManager();

	Level* advanceLevel();
};

