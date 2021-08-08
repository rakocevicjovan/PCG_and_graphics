#include "pch.h"

#include "TextureGen.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"


namespace Procedural
{

	float TextureGen::Perlin3D(float x, float  y, float z, UINT xw, UINT yw, UINT zw)
	{
		return stb_perlin_noise3(x, y, z, xw, yw, zw);
	}

	float TextureGen::Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves)
	{
		return stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves);
	}
	
	float TextureGen::Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves)
	{
		return stb_perlin_turbulence_noise3(x, y, z, lacunarity, gain, octaves);
	}

	float TextureGen::Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves)
	{
		return stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves);
	}

	std::vector<float> TextureGen::generateTurbulent(int w, int h, float z, float lacunarity, float gain, UINT octaves)
	{
		std::vector<float> result;
		result.reserve(w * h);

		float wInverse = 1.f / (float)w;
		float hInverse = 1.f / (float)h;

		for (int i = 0; i < w; ++i)
		{
			for (int j = 0; j < h; ++j)
			{
				float x = (float)i * wInverse;
				float y = (float)j * hInverse;

				float noiseVal = Turbulence3D(x, y, z, lacunarity, gain, octaves);
				result.push_back(noiseVal);
			}
		}

		return result;
	}

	std::vector<float> TextureGen::generateRidgey(int w, int h, float z, float lacunarity, float gain, float offset, UINT octaves)
	{
		std::vector<float> result;
		result.reserve(w * h);

		float wInverse = 1.f / (float)w;
		float hInverse = 1.f / (float)h;

		for (int i = 0; i < w; ++i)
		{
			for (int j = 0; j < h; ++j)
			{
				float x = (float)i * wInverse;
				float y = (float)j * hInverse;

				float noiseVal = Ridge3D(x, y, z, lacunarity, gain, offset, octaves);
				result.push_back(noiseVal);
			}
		}

		return result;
	}
}