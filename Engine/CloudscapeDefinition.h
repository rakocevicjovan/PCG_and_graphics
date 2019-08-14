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
		SVec4 repeat = SVec4(4096.f);
		SVec3 skyRGB = SVec3(135., 206., 250.) * (.8f / 255.f);
		SVec3 ALTop = SVec3(149., 167., 200.) * (1.2 / 255.);
		SVec3 ALBot = SVec3(39., 67., 87.) * (1.2 / 255.);

		Texture weather;
		Texture blue_noise;

		//3d texture resorce views
		ID3D11ShaderResourceView* baseVolume;
		ID3D11ShaderResourceView* fineVolume;
		Texture curlNoise;

		SVec3 rgb_sig_scattering;
		SVec3 rgb_sig_absorption;

		float eccentricity = 0.f;
		float scrQuadOffset = 15.f;
		float globalCoverage = .5f;
		float distanceLimit = 30000.f;
		float planetRadius = 6400000.f;
		float carvingThreshold = .5f;
		float textureSpan = .2f;
	};

}