#pragma once
#include "Level.h"
#include "ToolGUI.h"
#include "Terrain.h"
#include "Texture.h"
#include "CloudscapeDefinition.h"

#include <sstream>
#include <iomanip>


namespace Strife
{

	class StrifeLevel : public Level
	{
	public:
		StrifeLevel(Engine& sys) : Level(sys) {};
		~StrifeLevel() {};

		Model skybox, floor;
		OST sceneTex;
		ScreenspaceDrawer postProcessor;
		ScreenspaceDrawer::UINODE* screenRectangleNode;

		Model screenQuad;

		CloudscapeDefinition csDef;

		//load and draw all that jazz
		void init(Engine& sys);
		void procGen();
		void update(const RenderContext& rc);
		void draw(const RenderContext& rc);
		
		ID3D11ShaderResourceView* baseSrv;
		ID3D11ShaderResourceView* fineSrv;

		ID3D11Texture3D* baseTexId;
		ID3D11Texture3D* fineTexId;

		bool Create3D();
		bool CreateFine3D();
		bool Create3DOneChannel();
		bool CreateFine3DOneChannel();
	};
}