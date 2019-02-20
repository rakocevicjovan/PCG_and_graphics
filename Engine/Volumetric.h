#pragma once
#include <vector>
#include <d3d11.h>
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
		UINT _w, _h, _d;

		ID3D11Texture3D* _tex3d;
		ID3D11ShaderResourceView* _srv;

	public:
		
		Volumetric();
		~Volumetric();

		void init(UINT w = 10, UINT y = 10, UINT z = 10);
		inline UINT access(UINT x, UINT y, UINT z);
		void petrurb();

		void Create3DTexture(ID3D11Device* device);
	};
}