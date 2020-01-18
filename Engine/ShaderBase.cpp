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










bool ShaderBase::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	this->_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createCBufferDesc(sizeof(MatrixBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createCBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);

	return result;
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
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
	//deviceContext->PSSetShaderResources(1, 1, &(unbinder[0]));
	//deviceContext->PSSetShaderResources(2, 1, &(unbinder[0]));
	//deviceContext->PSSetShaderResources(3, 1, &(unbinder[0]));
}