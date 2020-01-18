#include "ShaderBase.h"
#include "Model.h"



ShaderBase::ShaderBase()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_lightBuffer = nullptr;
}


ShaderBase::~ShaderBase()
{
	DECIMATE(_vertexShader);
	DECIMATE(_pixelShader);
	DECIMATE(_layout);
	DECIMATE(_sampleState);
	DECIMATE(_matrixBuffer);
	DECIMATE(_lightBuffer);
}



bool ShaderBase::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	//common
	this->filePaths = filePaths;

	//common
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;
	ID3D10Blob* errorMessage = nullptr;
	////////////////////

	HRESULT result;

	//NOT common
	D3D11_BUFFER_DESC matrixBufferDesc, variableBufferDesc, lightBufferDesc;

	result = D3DCompileFromFile(filePaths.at(0).c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);

	if (FAILED(result)) 
	{
		if (errorMessage)	OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(0).c_str()));
		else	MessageBox(hwnd, filePaths.at(0).c_str(), L"Missing Shader File", MB_OK);
		return false;
	}

	result = D3DCompileFromFile(filePaths.at(1).c_str(), NULL, NULL, "main", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);

	if (FAILED(result)) 
	{
		if (errorMessage)	OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(1).c_str()));
		else	MessageBox(hwnd, filePaths.at(1).c_str(), L"Missing Shader File", MB_OK);
		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &_vertexShader);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &_pixelShader);
	if (FAILED(result))
		return false;

	result = device->CreateInputLayout(layoutDesc.data(), layoutDesc.size(), vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &_layout);
	if (FAILED(result))
		return false;

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	if (FAILED(device->CreateSamplerState(&samplerDesc, &_sampleState)))
		return false;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &_matrixBuffer)))
		return false;

	// Setup the description of the variable dynamic constant buffer that is in the vertex shader.
	variableBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	variableBufferDesc.ByteWidth = sizeof(VariableBuffer);
	variableBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	variableBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	variableBufferDesc.MiscFlags = 0;
	variableBufferDesc.StructureByteStride = 0;

	// Create the variable constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&variableBufferDesc, NULL, &_variableBuffer);
	if (FAILED(result))
		return false;


	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBuffer);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &_lightBuffer);
	if (FAILED(result))
		return false;

	return true;
}


void ShaderBase::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename)
{
	char* compileErrors = (char*)(errorMessage->GetBufferPointer());
	unsigned long bufferSize = errorMessage->GetBufferSize();

	std::ofstream fout;

	fout.open("shader-error.txt");
	
	for (unsigned long i = 0; i < bufferSize; i++)
		fout << compileErrors[i];

	fout.close();

	errorMessage->Release();
	errorMessage = nullptr;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", &shaderFilename, MB_OK);
}



bool ShaderBase::SetShaderParameters(ID3D11DeviceContext* dc, SMatrix& modelMat, const Camera& cam, const PointLight& pLight, float deltaTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	VariableBuffer* dataPtr3;
	LightBuffer* dataPtr2;

	SMatrix mT = modelMat.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();

	// Lock the constant matrix buffer so it can be written to.
	if (FAILED(dc->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;

	// Unlock the constant buffer.
	dc->Unmap(_matrixBuffer, 0);

	bufferNumber = 0;
	dc->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);


	//VARIABLE BUFFER
	if (FAILED(dc->Map(_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr3 = (VariableBuffer*)mappedResource.pData;
	dataPtr3->deltaTime = deltaTime;
	dataPtr3->padding = SVec3(); 
	dc->Unmap(_variableBuffer, 0);
	bufferNumber = 1;
	dc->VSSetConstantBuffers(bufferNumber, 1, &_variableBuffer);
	//END VARIABLE BUFFER


	// Lock the light constant buffer so it can be written to.
	if (FAILED(dc->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->pos = pLight.pos;
	dataPtr2->ePos = Math::fromVec3(cam.GetPosition(), 1.f);

	// Unlock the constant buffer.
	dc->Unmap(_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;
	dc->PSSetConstantBuffers(bufferNumber, 1, &_lightBuffer);

	dc->IASetInputLayout(_layout);
	dc->VSSetShader(_vertexShader, NULL, 0);
	dc->PSSetShader(_pixelShader, NULL, 0);
	dc->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void ShaderBase::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	//deviceContext->PSSetShaderResources(1, 1, &(unbinder[0]));
	//deviceContext->PSSetShaderResources(2, 1, &(unbinder[0]));
	//deviceContext->PSSetShaderResources(3, 1, &(unbinder[0]));
}