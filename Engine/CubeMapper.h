#pragma once

class Camera;

// This class has a confusing name, but todo renaming it. It allows rendering to all 6 faces of a cubemap. 
// It's not great code structure to have all this here, but it's currently not important as the codebase doesn't rely on it.
class CubeMapper
{
private:

	unsigned int _edgeLength{128};
	SMatrix _cameras[6];
	SMatrix _projMatrix;
	D3D11_VIEWPORT _viewport{};

	ID3D11Texture2D* _texPtr{};
	ID3D11ShaderResourceView* _shResView{};
	ID3D11RenderTargetView* _renderTargetViews[6]{};

	ID3D11Texture2D* _depthTexPtr{};
	ID3D11DepthStencilView* _depthStencilViews{};

	float _clearColour[4] = { 0.7f, 0.7f, 0.7f, 1.0f };

public:

	CubeMapper(const unsigned int edgeLength = 256);

	void init(ID3D11Device* dev);
	void updateCams(const SVec3& pos);
	void advance(ID3D11DeviceContext* dc, UINT i);
	Camera getCameraAtIndex(unsigned int i);

	inline ID3D11ShaderResourceView*& getShResView() { return _shResView; }
};