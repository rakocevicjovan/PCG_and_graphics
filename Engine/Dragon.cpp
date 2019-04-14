#include "Dragon.h"
#include "Systems.h"


void Dragon::init(const Model& head, const Model& segment)
{
	_head = head;
	_segment = segment;

	dud = new DragonUpdateData();

	pSys.init(&_segment, 10, SMatrix::CreateTranslation(SVec3(0, 50, 0)));

	particleUpdFunc = [this](ParticleUpdateData* pud) -> void
	{
		DragonUpdateData* dud = reinterpret_cast<DragonUpdateData*>(pud);

	#define head  pSys._particles[0]
		head->age += dud->dTime;

		SVec3 toPlayer = Math::getNormalizedVec3(dud->playerPos - head->transform.Translation());

		float angle = acos(head->transform.Forward().Dot(dud->playerPos - head->transform.Translation()));
		SVec3 cross = Math::getNormalizedVec3(head->transform.Forward().Cross(toPlayer));
		
		head->transform *= SMatrix::CreateFromAxisAngle(cross, angle * dud->dTime);
		Math::Translate(head->transform, toPlayer);
	#undef head

		for (int i = 1; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += dud->dTime;

			pSys._particles[i]->transform = pSys._particles[i - 1]->transform;

			//float windEffect = Math::smoothstep(0, pSys._particles.size(), i);
			SVec3 translation(dud->wind * dud->dTime * 1.f);
			Math::Translate(pSys._particles[i]->transform, translation);
		}
	};

	pSys.setUpdateFunction(particleUpdFunc);
}



void Dragon::update(const RenderContext& rc, const SVec3& wind)
{	
	dud->dTime = rc.dTime;
	dud->playerPos = rc.cam->GetPosition();
	pSys.pud = dud;

	pSys.update(rc.dTime);

	draw(rc);
}

void Dragon::draw(const RenderContext & rc)
{

}





/*
particleUpdFunc = [this](ParticleUpdateData* pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += pud->dTime * 0.1f;
			SVec3 translation(pud->windDirection * pud->windVelocity);	//
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * 0.33f);
		}
	};
*/