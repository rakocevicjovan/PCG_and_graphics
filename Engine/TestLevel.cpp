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
		t.SetUp(S_DEVICE);
		floorModel = Model(t, S_DEVICE);

		sm.loadModel(S_DEVICE, "../Models/Animated/Kachujin_walking/Walking.fbx");
		//sm.loadModel(device, "../Models/Animated/Bob/boblampclean.md5mesh");

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
		rc.d3d->setRSSolidNoCull();

		/* //adapt to the new system once it's done

		S_SHADY.light.SetShaderParameters(S_CONTEXT, floorModel.transform, *rc.cam, pLight);
		floorModel.Draw(S_CONTEXT, S_SHADY.light);

		sm.update(rc.dTime, bts, 0u);

		S_SHADY.animator.SetShaderParameters(S_CONTEXT, sm.transform, rc.cam->GetPosition(), pLight, bts);
		sm.Draw(S_CONTEXT, S_SHADY.animator);

		S_SHADY.light.ReleaseShaderParameters(S_CONTEXT);
		*/

		rc.d3d->EndScene();
	}
}

