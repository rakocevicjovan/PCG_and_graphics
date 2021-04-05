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
#include "CSM.h"
#include "RenderStage.h"
#include "FPSCounter.h"
#include "entt/entt.hpp"


struct Deleted
{
	bool whatever{false};
};


// Clean version of TDLevel without all the accumulated cruft.
class RenderingTestLevel : public Level
{
private:

	GeoClipmap _geoClipmap;
	Scene _scene;
	SceneEditor _sceneEditor;
	FPSCounter _fpsCounter;

	// Make it cozy in here for now, but move these to scene ultimately!
	DirectionalLight _dirLight;
	CBuffer _dirLightCB;
	CBuffer _positionBuffer;
	Skybox _skybox;

	std::vector<RenderStage> _stages;

public:

	RenderingTestLevel(Engine& sys)
		: Level(sys), _scene(_sys, AABB(SVec3(), SVec3(500.f * .5)), 5), _geoClipmap(3, 4, 10.), _fpsCounter(64)
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

		//S_RANDY._cam._controller->setFlying(true);

		Model* modelPtr = S_RESMAN.getByName<Model>("FlyingMage");
		auto vsPtr = sys._shaderCache.getVertShader("basicVS");
		auto psPtr = sys._shaderCache.getPixShader("phongPS");
		
		for (auto& mesh : modelPtr->_meshes)
		{
			mesh._material->setVS(vsPtr);
			mesh._material->setPS(psPtr);
		}

		auto _renderGroup = _scene._registry.group<CTransform, CStaticMesh>();

		for (UINT i = 0; i < 100; ++i)
		{
			auto entity = _scene._registry.create();
			_scene._registry.emplace<CStaticMesh>(entity, modelPtr);
			_scene._registry.emplace<CTransform>(entity, SMatrix::CreateTranslation(SVec3(i / 10, (i % 10), 10) * 10.f));
		}

		_sys._renderer._mainStage = RenderStage(
			S_RANDY.device(),
			&(S_RANDY._cam),
			&(S_RANDY.d3d()->_renderTarget),
			&(S_RANDY.d3d()->_viewport));
		/*
		RenderStage shadowStage(
			S_RANDY.device(),
			&(S_RANDY._cam),
			&(S_RANDY.d3d()->_renderTarget),
			&(S_RANDY.d3d()->_viewport));

		RenderStage mainStage(
			S_RANDY.device(),
			&(S_RANDY._cam),
			&(S_RANDY.d3d()->_renderTarget),
			&(S_RANDY.d3d()->_viewport));

		_stages.push_back(std::move(shadowStage));
		_stages.push_back(std::move(mainStage));
		*/

		_positionBuffer.init(S_RANDY.device(), CBuffer::createDesc(sizeof(SMatrix)));
		_positionBuffer.bindToVS(S_RANDY.context(), 0);
	}


	void fakeRenderSystem(ID3D11DeviceContext* context, entt::registry& registry)
	{
		auto group = _scene._registry.group<CTransform, CStaticMesh>();

		// Can try a single buffer for position

		//_positionBuffer.bindToVS(context, 0);

		//auto& mainPass = _sys._renderer._mainStage;
		//mainPass.bind(context, _sys._clock.deltaTime(), _sys._clock.totalTime());

		group.each([&context, &posBuffer = _positionBuffer](CTransform& transform, CStaticMesh& renderComp)
			{
				Model* model = renderComp.model;

				if (!model)
					return;

				posBuffer.bindToVS(context, 0);

				for (auto& mesh : model->_meshes)
				{
					mesh.bind(context);
					posBuffer.updateWithStruct(context, transform.transform.Transpose());

					Material* mat = mesh._material.get();
					mat->bind(context);

					// This is bad and needs to be rewritten...
					//mat->getVS()->updateBuffersAuto(context, mesh);
					//mat->getVS()->setBuffers(context);
					//mat->getPS()->updateBuffersAuto(context, mesh);
					//mat->getPS()->setBuffers(context);

					// Set shaders and textures.
					mat->bind(context);

					context->DrawIndexed(mesh._indexBuffer.getIdxCount(), 0, 0);
				}
			});
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

		fakeRenderSystem(rc.d3d->getContext(), _scene._registry);

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