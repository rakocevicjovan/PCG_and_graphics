#pragma once
#include "Math.h"
#include "Texture.h"

namespace Procedural
{

	struct Voxel
	{
		SVec3 pos;
		SVec4 col;
	};



	class Volumetric
	{
	protected:

		std::vector<Voxel> _volume;
		uint32_t _w{0u};
		uint32_t _h{0u};
		uint32_t _d{0u};

		ID3D11Texture3D* _tex3d;
		ID3D11ShaderResourceView* _arraySrv;

	public:

		void init(uint32_t w = 10u, uint32_t y = 10u, uint32_t z = 10u);
		inline uint32_t access(uint32_t x, uint32_t y, uint32_t z);
		void petrurb();

		void Create3DTexture(ID3D11Device* device);
	};
}