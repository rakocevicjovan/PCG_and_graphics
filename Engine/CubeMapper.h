#pragma once

#include <d3d11.h>
#include <vector>
#include "Math.h"

class Camera;

class CubeMapper
{
private:

	unsigned int _edgeLength;
	SMatrix _cameras[6];
	D3D11_VIEWPORT _viewport;
	SMatrix _projMatrix;

	ID3D11Texture2D *_texPtr;
	ID3D11ShaderResourceView* _shResView;
	ID3D11RenderTargetView* _renderTargetViews[6];

	ID3D11Texture2D* _depthTexPtr;
	ID3D11DepthStencilView* _depthStencilViews;

	float _clearColour[4] = { 0.7f, 0.7f, 0.7f, 1.0f };

public:

	CubeMapper(const unsigned int edgeLength = 256);
	~CubeMapper();

	void init(ID3D11Device* dev);
	void updateCams(const SVec3& pos);
	void advance(ID3D11DeviceContext* dc, UINT i);
	Camera getCameraAtIndex(unsigned int i);

	inline ID3D11ShaderResourceView*& getShResView() { return _shResView; }

	static void loadCubeMapFromFile(ID3D11Device* dev, const std::string& filename, UINT edgeLength, ID3D11Texture2D*& texPtr, ID3D11ShaderResourceView*& shResView);
	static D3D11_TEXTURE2D_DESC createCubeMapDescription(UINT edgeLength, bool renderTarget, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
};