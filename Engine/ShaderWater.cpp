#include "ShaderWater.h"
#include "Model.h"
#include "Camera.h"



ShaderWater::ShaderWater()
{
	m_vertexShader = nullptr;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	_matrixBuffer = 0;
	_lightBuffer = 0;
}



ShaderWater::~ShaderWater() {}



bool ShaderWater::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths)
{

	this->filePaths = filePaths;
	return InitializeShader(device, hwnd);
}



bool ShaderWater::InitializeShader(ID3D11Device* device, HWND hwnd)
{
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;

	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc, waterBufferDesc, clipperDesc;

	if (FAILED(D3DCompileFromFile(filePaths.at(0).c_str(), NULL, NULL, "strifeVertex", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage))) {
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(0).c_str()));
		else
			MessageBox(hwnd, filePaths.at(0).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}

	if (FAILED(D3DCompileFromFile(filePaths.at(1).c_str(), NULL, NULL, "strifeFragment", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage))) {
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(1).c_str()));
		else
			MessageBox(hwnd, filePaths.at(1).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}

	if (FAILED(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader)))
		return false;

	if (FAILED(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader)))
		return false;

	std::vector<D3D11_INPUT_ELEMENT_DESC> polyLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	if (FAILED(device->CreateInputLayout(polyLayout.data(), polyLayout.size(), vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout)))
		return false;

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	if (FAILED(device->CreateSamplerState(&samplerDesc, &m_sampleState)))
		return false;

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_matrixBuffer)))
		return false;

	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBuffer);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&waterBufferDesc, NULL, &_lightBuffer)))
		return false;

	return true;
}



void ShaderWater::ShutdownShader()
{
	DECIMATE(_lightBuffer)
	DECIMATE(_variableBuffer)
	DECIMATE(_matrixBuffer)
	DECIMATE(m_sampleState)
	DECIMATE(m_layout)
	DECIMATE(m_pixelShader)
	DECIMATE(m_vertexShader)
}



void ShaderWater::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename) {

	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++) {
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", &shaderFilename, MB_OK);
}



bool ShaderWater::SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& cam, const PointLight& pLight, float elapsed,
	ID3D11ShaderResourceView* whiteSRV, ID3D11ShaderResourceView* reflectionMap, ID3D11ShaderResourceView* refractionMap)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	WaterBuffer* dataPtr2;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();

	//SVec4 viewDir = -vT.Forward();	//@TODO CHECK IF THIS WORKS RIGHT!!! Inverse and transpose work similarly for view matrix... a bit of a hack tbh
	SVec4 viewDir = cam.GetCameraMatrix().Forward();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (WaterBuffer*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->dir = pLight.pos;
	dataPtr2->eyePos = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.f);
	dataPtr2->elapsed = elapsed;
	dataPtr2->padding = SVec3();
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);

	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetShaderResources(0, 1, &(whiteSRV));
	deviceContext->PSSetShaderResources(1, 1, &(reflectionMap));
	deviceContext->PSSetShaderResources(2, 1, &(refractionMap));
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	return true;
}



bool ShaderWater::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	deviceContext->PSSetShaderResources(1, 1, &(unbinder[0]));
	deviceContext->PSSetShaderResources(2, 1, &(unbinder[0]));
	return true;
}