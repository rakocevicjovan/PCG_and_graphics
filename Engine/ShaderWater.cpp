#include "ShaderWater.h"



ShaderWater::ShaderWater()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_waterBuffer = nullptr;
}



ShaderWater::~ShaderWater()
{
	ShutdownShader();
}



bool ShaderWater::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC waterBufferDesc = shc.createBufferDesc(sizeof(WaterBuffer));
	shc.createConstantBuffer(waterBufferDesc, _waterBuffer);

	return result;
}



void ShaderWater::ShutdownShader()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_matrixBuffer)
	DECIMATE(_waterBuffer)
	DECIMATE(_sampleState)
	DECIMATE(_layout)
}



bool ShaderWater::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SVec3& camPos, const PointLight& pLight, float elapsed,
	ID3D11ShaderResourceView* whiteSRV, ID3D11ShaderResourceView* reflectionMap, ID3D11ShaderResourceView* refractionMap)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	WaterBuffer* dataPtr2;

	SMatrix mT = mMat.Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	if (FAILED(deviceContext->Map(_waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (WaterBuffer*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->dir = pLight.pos;
	dataPtr2->eyePos = Math::fromVec3(camPos, 1.);
	dataPtr2->elapsed = elapsed;
	dataPtr2->padding = SVec3();
	deviceContext->Unmap(_waterBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_waterBuffer);


	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetShaderResources(0, 1, &(whiteSRV));
	deviceContext->PSSetShaderResources(1, 1, &(reflectionMap));
	deviceContext->PSSetShaderResources(2, 1, &(refractionMap));
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



bool ShaderWater::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
	deviceContext->PSSetShaderResources(1, 1, &(_unbinder[0]));
	deviceContext->PSSetShaderResources(2, 1, &(_unbinder[0]));
	return true;
}