#include "TestLevel.h"
#include "Terrain.h"

namespace SkelAnim
{

	void SkelAnimTestLevel::init(Systems& sys)
	{
		LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
		pLight = PointLight(lightData, SVec4(0, 500, 0, 1));

		float tSize = 200;
		Procedural::Terrain t(2, 2, SVec3(tSize));
		t.setOffset(-tSize * .5f, -10, -tSize * .5f);
		t.SetUp(device);
		floorModel = Model(t, device);

		sm.LoadModel(device, "../Models/Animated/Kachujin_walking/Walking.fbx");

		for (int i = 0; i < MAX_BONES; i++)
		{
			bts.emplace_back(SMatrix::Identity);
		}
	}



	void SkelAnimTestLevel::update(const RenderContext& rc)
	{
		ProcessSpecialInput(rc.dTime);
		updateCam(rc.dTime);
	}



	void SkelAnimTestLevel::draw(const RenderContext& rc)
	{
		rc.d3d->ClearColourDepthBuffers();
		rc.d3d->TurnOffCulling();

		shady.light.SetShaderParameters(context, floorModel.transform, *rc.cam, pLight, rc.dTime);
		floorModel.Draw(context, shady.light);

		sm.update(rc.dTime, bts, 0u);

		shady.animator.SetShaderParameters(context, sm, *rc.cam, pLight, rc.elapsed, bts);
		sm.Draw(context, shady.animator);

		shady.light.ReleaseShaderParameters(context);

		rc.d3d->EndScene();
	}
}

