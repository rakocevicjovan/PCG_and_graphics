#pragma once
#include "Level.h"
#include "ToolGUI.h"
#include "Terrain.h"
#include "CloudscapeDefinition.h"

namespace Strife
{

	class StrifeLevel : public Level
	{
	public:
		StrifeLevel(Systems& sys) : Level(sys) {};
		~StrifeLevel() {};

		Model skybox, floor;
		CubeMapper skyboxCubeMapper;

		Model screenQuad;

		CloudscapeDefinition csDef;

		//load and draw all that jazz
		void init(Systems& sys);
		void procGen();
		void update(const RenderContext& rc);
		void draw(const RenderContext& rc);

		void demolish()
		{
			this->~StrifeLevel();
		};
	};

}