#pragma once
#include "Texture.h"


namespace Procedural
{
	//wrapper for generation functions of a texture class, to keep the Texture:: interface cleaner...
	class TextureGen
	{
	public:

		inline static float Perlin3D(float x, float  y, float z, UINT xw = 0, UINT yw = 0, UINT zw = 0)
		{
			return Texture::Perlin3D(x, y, z, xw, yw, zw);
		}


		inline static float Perlin3DFBM(float x, float  y, float z, float lacunarity, float gain, UINT octaves)
		{
			return Texture::Perlin3DFBM(x, y, z, lacunarity, gain, octaves);
		}


		inline static float Turbulence3D(float x, float  y, float z, float lacunarity, float gain, UINT octaves)
		{
			return Texture::Turbulence3D(x, y, z, lacunarity, gain, octaves);
		}
		
		
		inline static float Ridge3D(float x, float  y, float z, float lacunarity, float gain, float offset, UINT octaves)
		{
			return Texture::Ridge3D(x, y, z, lacunarity, gain, offset, octaves);
		}

	};
}