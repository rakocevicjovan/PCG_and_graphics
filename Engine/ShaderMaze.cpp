#include "ShaderMaze.h"
#include "ShaderDataStructs.h"
#include "Texture.h"



ShaderMaze::ShaderMaze()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_matrixBuffer = nullptr;
	_mazePosBuffer = nullptr;
	_lightBuffer = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
}



ShaderMaze::~ShaderMaze()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_matrixBuffer)
	DECIMATE(_mazePosBuffer)
	DECIMATE(_lightBuffer)
	DECIMATE(_layout)
	DECIMATE(_sampleState)
}



bool ShaderMaze::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC mazePosBuffDesc = ShaderCompiler::createBufferDesc(sizeof(Vec4Buffer));
	result &= shc.createConstantBuffer(mazePosBuffDesc, _mazePosBuffer);
	
	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);

	return result;
}



bool ShaderMaze::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SVec3& camPos,
	const PointLight& pLight, float deltaTime, const Texture& diff, const Texture& n)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;
	Vec4Buffer* playerPosBuffer;

	//WMBuffer
	SMatrix mT = mMat.Transpose();
	
	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//MazePosBuffer
	SVec4 vec4camPos = Math::fromVec3(camPos, 1.f);

	if (FAILED(deviceContext->Map(_mazePosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	playerPosBuffer = (Vec4Buffer*)mappedResource.pData;
	playerPosBuffer->vec4 = vec4camPos;
	deviceContext->Unmap(_mazePosBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_mazePosBuffer);

	//LightBuffer
	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->pos = pLight.pos;
	dataPtr2->ePos = vec4camPos;
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	deviceContext->PSSetShaderResources(0, 1, &(diff._arraySrv));
	deviceContext->PSSetShaderResources(1, 1, &(n._arraySrv));

	return true;
}



void ShaderMaze::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
	deviceContext->VSSetShaderResources(1, 1, &(_unbinder[0]));
}