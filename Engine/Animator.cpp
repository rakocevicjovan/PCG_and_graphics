#include "Animator.h"
#include "SkeletalModel.h"
#include "Camera.h"

Animator::Animator()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	m_lightBuffer = 0;
}


Animator::~Animator()
{
}


bool Animator::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths)
{
	this->filePaths = filePaths;
	return InitializeShader(device, hwnd);
}


bool Animator::InitializeShader(ID3D11Device* device, HWND hwnd)
{

	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;

	//these store the shaders while they are being processed and are released after compilation... I think...
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;

	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC bonesBufferDesc;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(filePaths.at(0).c_str(), NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);

	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(0).c_str()));
		else
			MessageBox(hwnd, filePaths.at(0).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}

	result = D3DCompileFromFile(filePaths.at(1).c_str(), NULL, NULL, "main", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);

	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, *(filePaths.at(1).c_str()));
		else
			MessageBox(hwnd, filePaths.at(1).c_str(), L"Missing Shader File", MB_OK);

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	std::vector<D3D11_INPUT_ELEMENT_DESC> sbLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_ID" , 0, DXGI_FORMAT_R32G32B32A32_UINT,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_W"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the vertex input layout.
	result = device->CreateInputLayout(sbLayout.data(), 5, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
		&m_layout);
	if (FAILED(result))
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
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	// Setup the description of the variable dynamic constant buffer that is in the vertex shader.
	variableBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	variableBufferDesc.ByteWidth = sizeof(VariableBufferType);
	variableBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	variableBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	variableBufferDesc.MiscFlags = 0;
	variableBufferDesc.StructureByteStride = 0;

	// Create the variable constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&variableBufferDesc, NULL, &m_variableBuffer);
	if (FAILED(result))
		return false;

	bonesBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bonesBufferDesc.ByteWidth = sizeof(SMatrix) * MAX_BONES;
	bonesBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bonesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bonesBufferDesc.MiscFlags = 0;
	bonesBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&bonesBufferDesc, NULL, &m_bonesBuffer);
	if (FAILED(result))
		return false;


	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
		return false;

	for (int i = 0; i < MAX_BONES; ++i)
	{
		boneTransformBuffer.boneTransforms.emplace_back(SMatrix::Identity);
	}

	return true;
}



void Animator::ShutdownShader()
{
	// Release the light constant buffer.
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	// Release the variable constant buffer.
	if (m_variableBuffer)
	{
		m_variableBuffer->Release();
		m_variableBuffer = 0;
	}
	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

}



void Animator::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR shaderFilename)
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



bool Animator::SetShaderParameters(ID3D11DeviceContext* deviceContext, SkeletalModel& model, const Camera& c, const PointLight& pLight, float deltaTime, 
	const std::vector<SMatrix>& boneTransformsIn)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	VariableBufferType* dataPtr3;
	BoneTransformBufferType* dataPtr4;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = c.GetViewMatrix().Transpose();
	SMatrix pT = c.GetProjectionMatrix().Transpose();


	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
	deviceContext->Unmap(m_matrixBuffer, 0);
	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);


	result = deviceContext->Map(m_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr3 = (VariableBufferType*)mappedResource.pData;
	dataPtr3->deltaTime = deltaTime;
	dataPtr3->padding = SVec3();
	deviceContext->Unmap(m_variableBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_variableBuffer);

	result = deviceContext->Map(m_bonesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr4 = (BoneTransformBufferType*)mappedResource.pData;
	size_t toCopyLength = boneTransformsIn.size() * sizeof(SMatrix);
	memcpy(mappedResource.pData, boneTransformsIn.data(), toCopyLength);
	deviceContext->Unmap(m_bonesBuffer, 0);
	bufferNumber = 2;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_bonesBuffer);


	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr2 = (LightBufferType*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->pos = pLight.pos;
	dataPtr2->ePos = Math::fromVec3(c.GetPosition(), 1.0f);
	deviceContext->Unmap(m_lightBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);


	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	for (unsigned int i = 0; i < model.textures_loaded.size(); i++)
		deviceContext->PSSetShaderResources(i, 1, &(model.textures_loaded[i].srv));

	return true;
}



bool Animator::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	return true;
}