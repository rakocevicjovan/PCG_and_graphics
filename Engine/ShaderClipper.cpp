#include "ShaderClipper.h"
#include "Camera.h"


ShaderClipper::ShaderClipper()
{
}


ShaderClipper::~ShaderClipper()
{
	if (_clipperBuffer) { _clipperBuffer->Release(); _clipperBuffer = nullptr; }
}



bool ShaderClipper::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	this->_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);

	D3D11_BUFFER_DESC clipperBufferDesc = ShaderCompiler::createBufferDesc(sizeof(ClipperBuffer));
	result &= shc.createConstantBuffer(clipperBufferDesc, _clipperBuffer);

	return result;
}



bool ShaderClipper::SetClipper(ID3D11DeviceContext* deviceContext, const SVec4& clipper)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ClipperBuffer* dataPtr;

	if (FAILED(deviceContext->Map(_clipperBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (ClipperBuffer*)mappedResource.pData;
	dataPtr->clipper = clipper;
	deviceContext->Unmap(_clipperBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_clipperBuffer);

	return true;
}



bool ShaderClipper::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& pLight, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;

	SMatrix mT = mMat.Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &_clipperBuffer);

	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->pos = pLight.pos;
	dataPtr2->ePos = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.f);
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void ShaderClipper::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	//deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
}