#include "Dragon.h"
#include "Systems.h"



void Dragon::init(UINT segments, SVec3 initPos)
{
	for (int i = 0; i < segments; ++i)
		springs.emplace_back(initPos + i * SVec3(1, 0, 0) * restLength);

}



void Dragon::update(const RenderContext& rc, const SVec3& wind, SVec3 target)
{	
	dragonUpdData.dTime = rc.dTime;
	dragonUpdData.playerPos = target;
	dragonUpdData.wind = wind;
	dragonUpdData.speed = 1.f;

#pragma region headMovement 
	//head movement
	SVec3 toTarget = dragonUpdData.playerPos - springs[0].transform.Translation();
	float lenToPlayer = toTarget.Length();

	if (lenToPlayer > 0.0001f)
		toTarget.Normalize();
	else
		return;	//already caught the player, no need to update its game over anyways...
	
	SVec3 cross = springs[0].transform.Backward().Cross(toTarget);
	if (cross.LengthSquared() > 0.0001f)
	{
		cross.Normalize();
		SVec3 fw = springs[0].transform.Backward();
		SVec3 trUp = springs[0].transform.Up();

		float angle = acos(Math::clamp(-0.999999, 0.999999, fw.Dot(toTarget)));
		float upToGlobalUp = acos(Math::clamp(-0.999999, 0.999999, trUp.Dot(SVec3(0, 1, 0))));

		SQuat curOri = SQuat::CreateFromRotationMatrix(springs[0].transform);
		SQuat rotOri = SQuat::CreateFromAxisAngle(cross, angle);
		SQuat finalOri = SQuat::Concatenate(rotOri, curOri);

		//correct roll!!! YAY I MANAGED!
		SVec3 rollCorrecionAxis = trUp.Cross(SVec3(0, 1, 0));
		if (rollCorrecionAxis.LengthSquared() > 0.0001f)
		{
			rollCorrecionAxis.Normalize();
			SQuat rollCorrector = SQuat::CreateFromAxisAngle(rollCorrecionAxis, upToGlobalUp);
			finalOri = SQuat::Concatenate(rollCorrector, finalOri);
		}
		
		SQuat rotDelta = SQuat::Slerp(curOri, finalOri, rc.dTime * agility);

		Math::SetRotation(springs[0].transform, SMatrix::CreateFromQuaternion(rotDelta));
	}
	
	Math::Translate(springs[0].transform, springs[0].transform.Backward() * flyingSpeed * rc.dTime);
#pragma endregion headMovement

	massSpring();
}



void Dragon::massSpring()
{

	for (int i = 1; i < springs.size(); ++i)
	{
		//wind influence
		SVec3 translation(dragonUpdData.wind * dragonUpdData.dTime * 1.f);
		Math::Translate(springs[i].transform, translation);

		//spring influence
		SVec3 toPredecessor = springs[i - 1].transform.Translation() - springs[i].transform.Translation();

		SVec3 deltaVel = resolveSpring(toPredecessor, dragonUpdData.dTime);
		springs[i].vel += deltaVel;
		springs[i].vel *= friction;

		Math::Translate(springs[i].transform, springs[i].vel);
	}
}



SVec3 Dragon::resolveSpring(SVec3 toNeighbour, float dTime)
{
	float deltaDist = restLength - toNeighbour.Length();		

	SVec3 forceVec = Math::getNormalizedVec3(toNeighbour) * deltaDist;	//ma = -kd!

	SVec3 FS = -k * forceVec;

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