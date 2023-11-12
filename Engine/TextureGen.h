#pragma once
#include "Texture.h"


namespace Procedural
{
namespace TextureGen
{
		float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
		float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
		float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
		float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves);

		std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves);
		std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves);
}
}