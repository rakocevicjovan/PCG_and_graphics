#include "ShaderPT.h"



ShaderPT::ShaderPT()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_vpBuffer = nullptr;
	_lightBuffer = nullptr;
}



ShaderPT::~ShaderPT()
{
	ShutdownShader();
}



//uses ptn and wrap
bool ShaderPT::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	D3D11_BUFFER_DESC projBufferDesc = ShaderCompiler::createBufferDesc(sizeof(ViewProjBuffer));
	result &= shc.createConstantBuffer(projBufferDesc, _vpBuffer);

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);

	return result;
}



void ShaderPT::ShutdownShader()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_layout)
	DECIMATE(_sampleState)
	DECIMATE(_matrixBuffer)
	DECIMATE(_vpBuffer)
	DECIMATE(_lightBuffer)
}



bool ShaderPT::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SMatrix& vLight, 
	const SMatrix& pLight, const DirectionalLight& dLight, const SVec3& eyePos, ID3D11ShaderResourceView* projectionTexture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* wmBuffer;
	ViewProjBuffer* vpBuffer;
	LightBuffer* lightBufferPtr;

	//WMBuffer
	SMatrix mT = mMat.Transpose();

	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	wmBuffer = (WMBuffer*)mappedResource.pData;
	wmBuffer->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//VPBuffer
	SMatrix vLightT = vLight.Transpose();
	SMatrix pLightT = pLight.Transpose();

	result = deviceContext->Map(_vpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	vpBuffer = (ViewProjBuffer*)mappedResource.pData;
	vpBuffer->lightView = vLightT;
	vpBuffer->lightProjection = pLightT;
	deviceContext->Unmap(_vpBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_vpBuffer);

	//LightBuffer
	result = deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	lightBufferPtr = (LightBuffer*)mappedResource.pData;
	lightBufferPtr->alc = dLight.alc;
	lightBufferPtr->ali = dLight.ali;
	lightBufferPtr->dlc = dLight.dlc;
	lightBufferPtr->dli = dLight.dli;
	lightBufferPtr->slc = dLight.slc;
	lightBufferPtr->sli = dLight.sli;
	lightBufferPtr->pos = dLight.dir;
	lightBufferPtr->ePos = SVec4(eyePos.x, eyePos.y, eyePos.z, 1.0f);
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	deviceContext->PSSetShaderResources(0, 1, &projectionTexture);

	return true;
}



void ShaderPT::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}