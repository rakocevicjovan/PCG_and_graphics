#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <string>
#include "Math.h"
#include "lightclass.h"

class Model;

class ShaderShadow{

	struct MatrixBufferType {
		SMatrix world;
		SMatrix view;
		SMatrix projection;
		SMatrix lightView;
		SMatrix lightProjection;
	};

	struct LightBufferType {

		SVec3 alc;
		float ali;

		SVec3  dlc;
		float dli;

		SVec3 slc;
		float sli;

		SVec4 pos;

		SVec4 ePos;
	};

	struct LightBufferType2{
		SVec4 lightPosition;
	};

public:
	ShaderShadow();
	~ShaderShadow();

	bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths);
	bool InitializeShader(ID3D11Device*, HWND);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const SMatrix& v, const SMatrix& v2,
		const SMatrix& p, const SMatrix& p2, const PointLight& dLight, const SVec3& eyePos, ID3D11ShaderResourceView* projectionTexture);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR);

	ID3D11SamplerState *m_sampleStateClamp, *m_sampleStateWrap;

private:

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_variableBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_lightBuffer2;

	std::vector<std::wstring> filePaths;

	ID3D11ShaderResourceView* unbinder[1] = { nullptr };
};

