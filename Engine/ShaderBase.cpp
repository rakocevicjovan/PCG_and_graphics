#include "ShaderBase.h"
#include "Model.h"

#define DECIMATE(x) x->Release(); x = nullptr;

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
	D3D11_INPUT_ELEMENT_DESC* layout, unsigned int layoutSize, const D3D11_SAMPLER_DESC& samplerDesc) {

	this->filePaths = filePaths;

	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	//these store the shaders while they are being processed and are released after compilation... I think...
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;

	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	// Load and compile shader code
	result = D3DCompileFromFile(filePaths.at(0).c_str(), NULL, NULL, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);

	if (FAILED(result)) 
	{
		if (errorMessage)	OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(0).c_str()));
		else	MessageBox(hwnd, filePaths.at(0).c_str(), L"Missing Shader File", MB_OK);
		return false;
	}

	result = D3DCompileFromFile(filePaths.at(1).c_str(), NULL, NULL, "LightPixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);

	if (FAILED(result)) 
	{
		if (errorMessage)	OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(1).c_str()));
		else	MessageBox(hwnd, filePaths.at(1).c_str(), L"Missing Shader File", MB_OK);
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &_vertexShader);
	if (FAILED(result))
		return false;

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &_pixelShader);
	if (FAILED(result))
		return false;

	// Create the vertex input layout.
	result = device->CreateInputLayout(layout, layoutSize, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&_layout);
	if (FAILED(result))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &_sampleState);
	if (FAILED(result))
		return false;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &_matrixBuffer);
	if (FAILED(result))
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


void ShaderBase::ShutdownShader()
{
	// Release the light constant buffer.
	if (_lightBuffer) {
		_lightBuffer->Release();
		_lightBuffer = 0;
	}

	// Release the variable constant buffer.
	if (_variableBuffer) {
		_variableBuffer->Release();
		_variableBuffer = 0;
	}
	// Release the matrix constant buffer.
	if (_matrixBuffer) {
		_matrixBuffer->Release();
		_matrixBuffer = 0;
	}

	// Release the sampler state.
	if (_sampleState) {
		_sampleState->Release();
		_sampleState = 0;
	}

	// Release the layout.
	if (_layout) {
		_layout->Release();
		_layout = 0;
	}

	// Release the pixel shader.
	if (_pixelShader) {
		_pixelShader->Release();
		_pixelShader = 0;
	}

	// Release the vertex shader.
	if (_vertexShader) {
		_vertexShader->Release();
		_vertexShader = 0;
	}

}


void ShaderBase::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename) {

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
	errorMessage = nullptr;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", &shaderFilename, MB_OK);
}


bool ShaderBase::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	Model& model, const SMatrix& v, const SMatrix& p,
	const PointLight& dLight, const SVec3& eyePos, float deltaTime)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	LightBuffer* dataPtr2;
	VariableBuffer* dataPtr3;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = v.Transpose();
	SMatrix pT = p.Transpose();

	// Lock the constant matrix buffer so it can be written to.
	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.

	// Copy the matrices into the constant buffer.
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;

	// Unlock the constant buffer.
	deviceContext->Unmap(_matrixBuffer, 0);

	bufferNumber = 0;	// Set the position of the constant buffer in the vertex shader.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);	// Now set the constant buffer in the vertex shader with the updated values.
	//END MATRIX BUFFER



	//VARIABLE BUFFER
	result = deviceContext->Map(_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (VariableBuffer*)mappedResource.pData;

	// Copy the variablethe constant buffer.
	dataPtr3->deltaTime = deltaTime;
	dataPtr3->padding = SVec3(); //this is just padding so this data isn't used.

	// Unlock the variable constant buffer.
	deviceContext->Unmap(_variableBuffer, 0);

	// Set the position of the variable constant buffer in the vertex shader.
	bufferNumber = 1;

	// Now set the variable constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_variableBuffer);
	//END VARIABLE BUFFER


	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBuffer*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->alc = dLight.alc;
	dataPtr2->ali = dLight.ali;
	dataPtr2->dlc = dLight.dlc;
	dataPtr2->dli = dLight.dli;
	dataPtr2->slc = dLight.slc;
	dataPtr2->sli = dLight.sli;
	dataPtr2->pos = dLight.pos;
	dataPtr2->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);

	// Unlock the constant buffer.
	deviceContext->Unmap(_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	deviceContext->IASetInputLayout(_layout);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &_lightBuffer);

	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	//if(model.textures_loaded.size() != 0)
	for (int i = 0; i < model.textures_loaded.size(); i++) {
		deviceContext->PSSetShaderResources(0, 1, &(model.textures_loaded[i].srv));
	}


	return true;
}



bool ShaderBase::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext) {
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	return true;
}