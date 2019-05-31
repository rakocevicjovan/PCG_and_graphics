#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "CloudscapeDefinition.h"

class Model;
class Camera;

class ShaderStrife
{

	struct CloudBuffer
	{
		SVec4 lightPos;
		SVec4 lightColInt;
		SVec4 eyePosElapsed;
		SVec4 eccentricity;

		SMatrix lightView;
	};

public:
	ShaderStrife();
	~ShaderStrife();

	bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths);
	bool InitializeShader(ID3D11Device*, HWND);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& cam, const Strife::CloudscapeDefinition& csDef, float elapsed);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR);

	ID3D11SamplerState* m_sampleState;

	RenderFormat renderFormat;
	UINT texturesAdded = 1;

private:

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_variableBuffer;
	ID3D11Buffer* cloudBuffer;

	std::vector<std::wstring> filePaths;

	ID3D11ShaderResourceView* unbinder[1] = { nullptr };
};

