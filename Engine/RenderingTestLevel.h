#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "Scene.h"
#include "NavGrid.h"
#include "AStar.h"
#include "Skybox.h"
#include "GUI.h"
#include "SceneEditor.h"
#include "TDController.h"
#include "FPSCounter.h"

// Clean version of TDLevel without all the accumulated cruft.
class RenderingTestLevel : public Level
{
private:

	GeoClipmap _geoClipmap;
	Scene _scene;
	SceneEditor _sceneEditor;
	FPSCounter _fpsCounter;

	TDController _tdController;

	// Make it cozy in here for now, but move these to scene ultimately!
	DirectionalLight _dirLight;
	CBuffer _dirLightCB;
	Skybox _skybox;

public:

	RenderingTestLevel(Engine& sys)
		: Level(sys), _scene(_sys, AABB(SVec3(), SVec3(500.f * .5)), 5), _geoClipmap(3, 4, 10.)
	{

	}

	void init(Engine& sys) override final
	{
		_sys._resMan.loadBatch(PROJ.getProjDir(), PROJ.getLevelReader().getLevelResourceDefs());	// This actually is data driven :)
		_sys._shaderCache.createAllShadersBecauseIAmTooLazyToMakeThisDataDriven();
		_sys._matCache.createAllMaterialsBecauseIAmTooLazyToMakeThisDataDriven();

		// All of this should not have to be here! Goal of this refactor is to kill it.
		Model* skyBoxModel = S_RESMAN.getByName<Model>("Skysphere");
		_skybox = Skybox(S_DEVICE, "../Textures/day.dds", skyBoxModel, S_MATCACHE.getMaterial("skybox"));

		LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);

		_dirLight = DirectionalLight(lightData, SVec4(0, -1, 0, 0));
		_dirLight.createCBuffer(S_DEVICE, _dirLightCB);
		_dirLight.updateCBuffer(S_CONTEXT, _dirLightCB);

		_geoClipmap.init(S_DEVICE);

		_sceneEditor.init(&_scene);

		_scene._csm.init(S_DEVICE, 1024u, 1024u, S_SHCACHE.getVertShader("csmVS"));

		S_INMAN.registerController(&_tdController);
		S_RANDY._cam._controller->setFlying(true);
	}

	void update(const RenderContext& rc) override final
	{
		_scene.update();
		_fpsCounter.tickFast(rc.dTime);
	}

	void draw(const RenderContext& rc) override final
	{
		_dirLight.bind(S_CONTEXT, _dirLightCB);

		_scene.draw();

		S_RANDY.d3d()->setRSWireframe();
		_geoClipmap.draw(S_CONTEXT);
		S_RANDY.d3d()->setRSSolidCull();

		_skybox.renderSkybox(*rc.cam, S_RANDY);

		GUI::beginFrame();

		_sceneEditor.display();

		std::vector<GuiElement> guiElems =
		{
			{"Octree",	std::string("OCT node count " + std::to_string(_scene._octree.getNodeCount()))},
			{"Octree",	std::string("OCT hull count " + std::to_string(_scene._octree.getHullCount()))},
			{"FPS",		std::string("FPS: " + std::to_string(_fpsCounter.getAverageFPS()))},
			{"Culling", std::string("Objects culled:" + std::to_string(_scene._numCulled))}
		};
		GUI::renderGuiElems(guiElems);

		GUI::endFrame();

		rc.d3d->EndScene();
	}

};