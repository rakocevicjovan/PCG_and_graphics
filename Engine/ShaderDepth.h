#pragma once
#include "Math.h"
#include <d3d11.h>
#include <vector>

class Model;

class ShaderDepth{

	struct MatrixBufferType{
		SMatrix world;
		SMatrix view;
		SMatrix projection;
	};


public:
	ShaderDepth();
	~ShaderDepth();


bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring>);
bool SetShaderParameters(ID3D11DeviceContext*, Model&, const SMatrix&, const SMatrix&);
void ShaderDepth::ShutdownShader();

private:
	bool InitializeShader(ID3D11Device*, HWND);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR);
	
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	std::vector<std::wstring> filePaths;
};

