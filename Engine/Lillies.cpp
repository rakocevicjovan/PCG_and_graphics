#include "Lillies.h"
#include "Systems.h"
#include "Model.h"
#include "Light.h"


Lillies::Lillies() {}

Lillies::~Lillies() {}


void Lillies::init(float lillyRadius, float rotationRate, SVec2 edges, SVec3 center)
{
	_lillyRadius = lillyRadius;
	_rotationRate = rotationRate;
	_edges = edges;
	_midPoint = center;

	_ringThickness = 2.f * lillyRadius;
	_numRings = floor((edges.y - edges.x) / _ringThickness);

	_lillyRings.reserve(_numRings);	//assume its bigger than it is

	//inside out, ring by ring
	for (int i = 0; i < _numRings; ++i)
	{
		float distToCenter = edges.x + (float(i) + 0.5f) * _ringThickness;	//ring's midline to inner circle
		float ringCircum = 2.0f * distToCenter * PI;

		_lillyRings.emplace_back(distToCenter);
		_lillyRings.back().rotSpeed = (i % 2) == 0 ? _rotationRate : -_rotationRate;
		_lillyRings.back().rotSpeed *= Math::smoothstep(_numRings, 0, i);
			
		UINT numLilliesInRing = floor(ringCircum / _ringThickness);
		_numLillies += numLilliesInRing;
		_lillyRings[i]._lillies.reserve(numLilliesInRing);

		float radianLillyOffset = 2 * PI / float(numLilliesInRing);

		float rowOffset = radianLillyOffset * .5f;

		for (int j = 0; j < numLilliesInRing; ++j)
		{
			SMatrix transform = 
				SMatrix::CreateTranslation(
					center + 
					SVec3(
					cos(radianLillyOffset * j + rowOffset),
					0, 
					-sin(radianLillyOffset * j + rowOffset)
					) 
					* distToCenter);
			_lillyRings[i]._lillies.emplace_back(transform, i);
		}
	}

	instanceData.reserve(_numLillies);
	luckOfTheDraw();
}



void Lillies::luckOfTheDraw()
{
	Chaos c;
	for (Ring& ring : _lillyRings)
	{
		bool connected = false;
		
		for (Lilly& lilly : ring._lillies)
			if (c.rollTheDice() > .66f)
				lilly.real = connected = true;

		//assure at least one is real in order to connect all rings... can't let lady luck do it all
		if (!connected)
			ring._lillies[c.rollTheDice() * (ring._lillies.size() - 1)].real = true;
	}
}



void Lillies::update(float dTime)
{
	for (Ring& ring : _lillyRings)
	{
		SMatrix rotMatrix = SMatrix::CreateFromAxisAngle(SVec3::Up, ring.rotSpeed * dTime);
		for (Lilly& lilly : ring._lillies)
		{
			Math::Scale(lilly.act.transform, SVec3(33.333333));
			Math::RotateMatByMat(lilly.act.transform, rotMatrix);
			lilly.act.collider->transform = lilly.act.transform;
		}
			
	}
		
	instanceData.clear();

	for (Ring& ring : _lillyRings)
		for (Lilly& lilly : ring._lillies)
			instanceData.emplace_back(lilly.act.transform.Transpose());
}



void Lillies::draw(const RenderContext & rc, Model& lillyModel, const PointLight& pLight, bool isFiltering)
{
	rc.shMan->instanced.UpdateInstanceData(instanceData);
	rc.shMan->instanced.SetShaderParameters(rc.d3d->GetDeviceContext(), lillyModel, *rc.cam, pLight, rc.dTime);
	lillyModel.Draw(rc.d3d->GetDeviceContext(), rc.shMan->instanced);
	rc.shMan->instanced.ReleaseShaderParameters(rc.d3d->GetDeviceContext());
}
