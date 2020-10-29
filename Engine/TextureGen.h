#pragma once
#include "Texture.h"


namespace Procedural
{
	class TextureGen
	{
	public:

		inline static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0);
		inline static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
		inline static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves);
		inline static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves);

		static std::vector<float> generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves);
		static std::vector<float> generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves);
	};
}