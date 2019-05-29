#pragma once
#include "Level.h"

namespace Strife
{

	class StrifeLevel : public Level
	{
	public:
		StrifeLevel(Systems& sys) : Level(sys) {};
		~StrifeLevel() {};

		PointLight pointLight;
		DirectionalLight dirLight;

		Model skybox;
		CubeMapper skyboxCubeMapper;

		Texture mazeDiffuseMap, mazeNormalMap;

		//load and draw all that jazz
		void init(Systems& sys);
		void procGen() {};
		void update(const RenderContext& rc);
		void draw(const RenderContext& rc);

		void demolish()
		{
			this->~StrifeLevel();
		};
	};

}