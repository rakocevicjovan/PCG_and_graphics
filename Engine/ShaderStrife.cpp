#include "ShaderStrife.h"
#include "Model.h"
#include "Camera.h"


ShaderStrife::ShaderStrife()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	cloudBuffer = 0;
}



ShaderStrife::~ShaderStrife()
{
}



bool ShaderStrife::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths)
{
	this->filePaths = filePaths;
	return InitializeShader(device, hwnd);
}



bool ShaderStrife::InitializeShader(ID3D11Device* device, HWND hwnd)
{
	ID3D10Blob* errorMessage = nullptr;

	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;

	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cloudBufferDesc;

	
	if (FAILED(D3DCompileFromFile(filePaths.at(0).c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage))) {
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(0).c_str()));
		else
			MessageBox(hwnd, filePaths.at(0).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}

	if (FAILED(D3DCompileFromFile(filePaths.at(1).c_str(), NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage))) {
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(1).c_str()));
		else
			MessageBox(hwnd, filePaths.at(1).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}
	

	// Create the vertex shader from the buffer.
	if (FAILED(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader)))
		return false;

	// Create the pixel shader from the buffer.
	if (FAILED(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader)))
		return false;
	
	/*
	if (FAILED(D3DReadFileToBlob(L"strifevs.hlsl", &vertexShaderBuffer)))
		OutputShaderErrorMessage(vertexShaderBuffer, hwnd, *(filePaths.at(0).c_str()));

	if (FAILED(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader)))
		assert(false);

	if(FAILED(D3DReadFileToBlob(L"strifeps.hlsl", &pixelShaderBuffer)))
		OutputShaderErrorMessage(pixelShaderBuffer, hwnd, *(filePaths.at(1).c_str()));

	if (FAILED(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader)))
		assert(false);
	*/

	std::vector<D3D11_INPUT_ELEMENT_DESC> sbLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
		//{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED(device->CreateInputLayout(sbLayout.data(), sbLayout.size(), vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout)))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 8;
	if (FAILED(device->CreateSamplerState(&samplerDesc, &m_sampleState)))
		return false;

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer)))
		return false;

	cloudBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cloudBufferDesc.ByteWidth = sizeof(CloudBuffer);
	cloudBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cloudBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cloudBufferDesc.MiscFlags = 0;
	cloudBufferDesc.StructureByteStride = 0;
	if (FAILED(device->CreateBuffer(&cloudBufferDesc, NULL, &cloudBuffer)))
		return false;

	return true;
}



void ShaderStrife::ShutdownShader()
{
	DECIMATE(cloudBuffer)
	DECIMATE(m_variableBuffer)
	DECIMATE(m_matrixBuffer)
	DECIMATE(m_sampleState)
	DECIMATE(m_layout)
	DECIMATE(m_pixelShader)
	DECIMATE(m_vertexShader)
}



void ShaderStrife::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");
	for (i = 0; i < bufferSize; i++)
		fout << compileErrors[i];

	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", &shaderFilename, MB_OK);
}



bool ShaderStrife::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& cam, const Strife::CloudscapeDefinition& csDef, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	CloudBuffer* dataPtr2;

#define SCREENSPACE true

#if SCREENSPACE
	SMatrix mT, vT, pT;
	mT = vT = pT = SMatrix::Identity;
#else
	SMatrix mT = csDef.planeMat.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();
#endif

	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	



	if (FAILED(deviceContext->Map(cloudBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (CloudBuffer*)mappedResource.pData;

	dataPtr2->lightPos = csDef.celestial.pos * 100000.f;
	dataPtr2->lightRGBI = Math::fromVec3(csDef.celestial.alc, csDef.celestial.ali);
	dataPtr2->extinction = Math::fromVec3(csDef.rgb_sig_absorption, 1.f - csDef.globalCoverage);
	dataPtr2->eyePosElapsed = Math::fromVec3(cam.GetPosition(), elapsed);
	dataPtr2->eccentricity = SVec4(csDef.eccentricity, csDef.heightMask.x, csDef.heightMask.y, csDef.scrQuadOffset);
	dataPtr2->repeat = csDef.repeat;

	dataPtr2->camMatrix = cam.GetCameraMatrix().Transpose();

	deviceContext->Unmap(cloudBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &cloudBuffer);

	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->PSSetShaderResources(0, 1, &(csDef.coverage_broad.srv));
	deviceContext->PSSetShaderResources(1, 1, &(csDef.coverage_frequent.srv));
	deviceContext->PSSetShaderResources(2, 1, &(csDef.blue_noise.srv));
	deviceContext->PSSetShaderResources(3, 1, &(csDef.baseShape));
	return true;
}



bool ShaderStrife::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	return true;
}