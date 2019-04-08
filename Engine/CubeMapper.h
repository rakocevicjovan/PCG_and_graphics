#pragma once

#include <d3d11.h>
#include <vector>
#include "Math.h"

class Camera;

struct Colour
{
	unsigned char col[4];

	Colour(int r, int g, int b, int a)
	{
		col[0] = (unsigned char)r;
		col[1] = (unsigned char)g;
		col[2] = (unsigned char)b;
		col[3] = (unsigned char)a;
	}
};

class CubeMapper{

public:
	CubeMapper(const unsigned int edgeLength = 256);
	~CubeMapper();

	unsigned int edgeLength;
	void Init(ID3D11Device* device);
	void UpdateCams(const SVec3& pos);
	void Advance(ID3D11DeviceContext* dc, UINT i);
	void LoadFromFiles(ID3D11Device* device, const std::string& filename);	//std::vector<std::string>& filenames
	Camera getCameraAtIndex(unsigned int i);

	SMatrix cameras[6];
	D3D11_VIEWPORT cm_viewport;
	SMatrix lens;
	ID3D11Texture2D *cm_id, *cm_depth_id;
	ID3D11ShaderResourceView* cm_srv;
	ID3D11RenderTargetView* cm_rtv[6];
	ID3D11DepthStencilView* cm_depthStencilView;

	float clearCol[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
};

