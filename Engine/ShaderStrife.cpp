#include "ShaderStrife.h"
#include "Camera.h"


ShaderStrife::ShaderStrife()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_cloudBuffer = nullptr;
}



ShaderStrife::~ShaderStrife()
{
	ShutdownShader();
}



//uses ptn layout and custom sample state
bool ShaderStrife::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC cloudBufferDesc = ShaderCompiler::createBufferDesc(sizeof(CloudBuffer));
	result &= shc.createConstantBuffer(cloudBufferDesc, _cloudBuffer);

	return result;
}



void ShaderStrife::ShutdownShader()
{
	DECIMATE(_cloudBuffer)
	DECIMATE(_matrixBuffer)
	DECIMATE(_sampleState)
	DECIMATE(_layout)
	DECIMATE(_pixelShader)
	DECIMATE(_vertexShader)
}



bool ShaderStrife::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& cam, const Strife::CloudscapeDefinition& csDef, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	CloudBuffer* dataPtr2;

#define SCREENSPACE true
#if SCREENSPACE
	SMatrix mT = SMatrix::Identity;
#else
	SMatrix mT = csDef.planeMat.Transpose();
#endif

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);	


	if (FAILED(deviceContext->Map(_cloudBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (CloudBuffer*)mappedResource.pData;
	dataPtr2->lightPos = csDef.celestial.pos * 99999999.f;
	dataPtr2->lightRGBI = Math::fromVec3(csDef.celestial.alc, csDef.celestial.ali);
	dataPtr2->extinction = Math::fromVec3(csDef.rgb_sig_absorption, 1.f - csDef.globalCoverage);
	dataPtr2->eyePosElapsed = Math::fromVec3(cam.GetPosition(), elapsed);
	dataPtr2->eccentricity = SVec4(csDef.eccentricity, csDef.heightMask.x, csDef.heightMask.y, csDef.scrQuadOffset);
	dataPtr2->repeat = csDef.repeat;
	dataPtr2->opt = SVec4(1.f / (csDef.heightMask.y - csDef.heightMask.x),
		1.f / csDef.repeat.x,
		1.f / csDef.repeat.y,
		csDef.planetRadius);
	dataPtr2->misc = Math::fromVec3(csDef.skyRGB, csDef.distanceLimit);
	dataPtr2->ALTop = Math::fromVec3(csDef.ALTop, csDef.carvingThreshold);
	dataPtr2->ALBot = Math::fromVec3(csDef.ALBot, csDef.textureSpan);
	dataPtr2->camMatrix = cam.GetCameraMatrix().Transpose();
	deviceContext->Unmap(_cloudBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_cloudBuffer);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	deviceContext->PSSetShaderResources(0, 1, &(csDef.weather._arraySrv));
	deviceContext->PSSetShaderResources(1, 1, &(csDef.blue_noise._arraySrv));
	deviceContext->PSSetShaderResources(2, 1, &(csDef.baseVolume));
	deviceContext->PSSetShaderResources(3, 1, &(csDef.fineVolume));

	return true;
}



void ShaderStrife::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
	deviceContext->PSSetShaderResources(1, 1, &(_unbinder[0]));
	deviceContext->PSSetShaderResources(2, 1, &(_unbinder[0]));
	deviceContext->PSSetShaderResources(3, 1, &(_unbinder[0]));
}