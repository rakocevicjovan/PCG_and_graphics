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

class ShaderWater {

	struct MatrixBufferType {
		SMatrix world;
		SMatrix view;
		SMatrix projection;
	};


	struct LightBufferType {

		SVec3 alc;
		float ali;

		SVec3  dlc;
		float dli;

		SVec3 slc;
		float sli;

		SVec4 dir;

		SVec4 eyePos;

		float elapsed;
		SVec3 padding;
	};

public:
	ShaderWater();
	~ShaderWater();

	bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths);
	bool InitializeShader(ID3D11Device*, HWND);
	bool SetShaderParameters(ID3D11DeviceContext*, Model& m, const SMatrix& v, const SMatrix& p,
		const DirectionalLight& dirLight, const SVec3& eyePos, float deltaTime,
		ID3D11ShaderResourceView* whiteSRV);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR);

	ID3D11SamplerState* m_sampleState;

private:

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_variableBuffer;
	ID3D11Buffer* m_lightBuffer;

	std::vector<std::wstring> filePaths;

	float timeElapsed;

	ID3D11ShaderResourceView* unbinder[1] = { nullptr };
};

