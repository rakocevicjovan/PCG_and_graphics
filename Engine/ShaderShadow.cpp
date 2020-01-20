#include "ShaderShadow.h"
#include "Model.h"



ShaderShadow::ShaderShadow()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleStateWrap = nullptr;
	_sampleStateClamp = nullptr;
	_matrixBuffer = nullptr;
	_shadowMatrixBuffer = nullptr;
	_lightBuffer = nullptr;
}



ShaderShadow::~ShaderShadow()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_layout)
	DECIMATE(_sampleStateWrap)
	DECIMATE(_sampleStateClamp)
	DECIMATE(_matrixBuffer)
	DECIMATE(_shadowMatrixBuffer)
	DECIMATE(_lightBuffer)
}



bool ShaderShadow::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);

	// First sample state is the usual, other one has clamping on all axes
	result &= shc.createSamplerState(samplerDesc, _sampleStateWrap);
	result &= shc.createSamplerState(samplerDesc, _sampleStateClamp);

	
	D3D11_BUFFER_DESC matrixBufferDesc = shc.createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(matrixBufferDesc, _matrixBuffer);

	D3D11_BUFFER_DESC shadowMatrixBufferDesc = shc.createBufferDesc(sizeof(ShadowMatrixBuffer));
	result &= shc.createConstantBuffer(shadowMatrixBufferDesc, _shadowMatrixBuffer);

	D3D11_BUFFER_DESC lightBufferDesc = shc.createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBufferDesc, _lightBuffer);

	return result;
}



bool ShaderShadow::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SMatrix& v2,
	const SMatrix& p2, const PointLight& pLight, const SVec3& eyePos, ID3D11ShaderResourceView* depthMapTexture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	WMBuffer* wmBuffer;
	ShadowMatrixBuffer* shadowMatBuffer;
	LightBuffer* lightBuffer;

	SMatrix mT = mMat.Transpose();

	//BEGIN WORLD MATRIX BUFFER
	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	wmBuffer = (WMBuffer*)mappedResource.pData;
	wmBuffer->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	//
	
	//BEGIN SHADOW BUFFER
	SMatrix v2T = v2.Transpose();
	SMatrix p2T = p2.Transpose();
	
	result = deviceContext->Map(_shadowMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	shadowMatBuffer = (ShadowMatrixBuffer*)mappedResource.pData;
	shadowMatBuffer->lightView = v2T;
	shadowMatBuffer->lightProjection = p2T;
	deviceContext->Unmap(_shadowMatrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_shadowMatrixBuffer);
	//


	//BEGIN LIGHT BUFFER
	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	lightBuffer = (LightBuffer*)mappedResource.pData;
	lightBuffer->alc = pLight.alc;
	lightBuffer->ali = pLight.ali;
	lightBuffer->dlc = pLight.dlc;
	lightBuffer->dli = pLight.dli;
	lightBuffer->slc = pLight.slc;
	lightBuffer->sli = pLight.sli;
	lightBuffer->pos = pLight.pos;
	lightBuffer->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);
	//

	deviceContext->IASetInputLayout(_layout);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &_sampleStateWrap);
	deviceContext->PSSetSamplers(1, 1, &_sampleStateClamp);

	deviceContext->PSSetShaderResources(0, 1, &depthMapTexture);

	return true;
}