#pragma once
#include "Math.h"
#include "Texture.h"
#include "Light.h"

namespace Strife
{

	class CloudscapeDefinition
	{
	public:
		PointLight celestial;

		SVec2 heightMask;

		SMatrix planeMat;
		SMatrix lightViewMat;

		Texture coverage_broad;
		//one for whole map pretty much
		//half precision float data type, channel for presence (density), channel for rainyness

		Texture coverage_frequent;
		//probably to be used tiled, or will devour memory...
		//used to disturb flat gradient made by height mask, two full precision float channels, top and bottom?

		Texture blue_noise;

		//3d texture resorce views
		ID3D11ShaderResourceView* SRV3D;

		SVec3 rgb_sig_scattering;
		SVec3 rgb_sig_absorption;

		float eccentricity = 0.f;

		float scrQuadOffset = 15.f;

		float globalCoverage = 1.f;
	};

}