#include "Dragon.h"
#include "Systems.h"



void Dragon::init(UINT segments, SVec3 initPos)
{
	for (int i = 0; i < segments; ++i)
	{
		springs.emplace_back(initPos + i * SVec3(1, 0, 0) * restLength);
	}
	
}



void Dragon::update(const RenderContext& rc, const SVec3& wind)
{	
	dragonUpdData.dTime = rc.dTime;
	dragonUpdData.playerPos = rc.cam->GetPosition();
	dragonUpdData.wind = wind;
	dragonUpdData.speed = 1.f;

	
	//head movement
	SVec3 toPlayer = dragonUpdData.playerPos - springs[0].transform.Translation();
	float lenToPlayer = toPlayer.Length();

	if (lenToPlayer > 0.0001f)
		toPlayer.Normalize();
	else
		return;	//already caught the player, no need to update its game over anyways...
	
	SVec3 cross = springs[0].transform.Backward().Cross(toPlayer);
	if (cross.LengthSquared() > 0.0001f)
	{
		cross.Normalize();
		SVec3 fw = springs[0].transform.Backward();
		SVec3 trUp = springs[0].transform.Up();

		float angle = acos(Math::clamp(0.000001, 0.999999, fw.Dot(toPlayer)));
		float upToGlobalUp = acos(Math::clamp(0.000001, 0.999999, trUp.Dot(SVec3(0, 1, 0))));

		//correct roll somehow...
		SQuat curOri = SQuat::CreateFromRotationMatrix(springs[0].transform);
		SQuat rotOri = SQuat::CreateFromAxisAngle(cross, angle);
		SQuat finalOri = SQuat::Concatenate(rotOri, curOri);

		//SQuat rollCorrector = SQuat::CreateFromAxisAngle(fw, upToGlobalUp);
		//finalOri = SQuat::Concatenate(finalOri, rollCorrector);
		
		SQuat rotDelta = SQuat::Slerp(curOri, finalOri, rc.dTime);

		//springs[0].transform = SMatrix::Transform(springs[0].transform, rotDelta);
		Math::SetRotation(springs[0].transform, SMatrix::CreateFromQuaternion(rotDelta));
	}

	Math::Translate(springs[0].transform, toPlayer * rc.dTime * flyingSpeed);

	massSpring();
}



void Dragon::massSpring()
{

	for (int i = 1; i < springs.size(); ++i)
	{
		//wind influence
		//SVec3 translation(dragonUpdData.wind * dragonUpdData.dTime * 1.f);
		//Math::Translate(springs[i].transform, translation);


		//spring influence
		SVec3 toPredecessor(springs[i - 1].transform.Translation() - springs[i].transform.Translation());

		SVec3 deltaVel = resolveSpring(toPredecessor, dragonUpdData.dTime);
		springs[i].vel += deltaVel;

		Math::Translate(springs[i].transform, springs[i].vel);
	}
}



SVec3 Dragon::resolveSpring(SVec3 toNeighbour, float dTime)
{
	float deltaDist = toNeighbour.Length() - restLength;	// deltaDist < 0 when compressed, > 0 when elongated

	SVec3 forceVec = Math::getNormalizedVec3(toNeighbour) * deltaDist;

	SVec3 FS = -stiffness * forceVec;

	SVec3 accel = FS * invMass;	//assume all masses are the same; a = dv / dt 

	SVec3 dVel = accel * dTime;

	return dVel;
}



/*
particleUpdFunc = [this](ParticleUpdateData* _pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += _pud->dTime * 0.1f;
			SVec3 translation(_pud->windDirection * _pud->windVelocity);	//
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * 0.33f);
		}
	};
*/