#include "TerrainShader.h"
#include "MeshDataStructs.h"
#include "Camera.h"


TerrainShader::TerrainShader()
{
}



TerrainShader::~TerrainShader()
{
}



bool TerrainShader::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);

	return result;
}



void TerrainShader::SetShaderParameters(ID3D11DeviceContext* dc, const SMatrix& mt, const SVec3& camPos, const PointLight& pointLight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;

	SMatrix mT = mt.Transpose();

	if (FAILED(dc->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return;
	dataPtr = (WMBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	dataPtr->world = mT;
	dc->Unmap(_matrixBuffer, 0);
	dc->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	if (FAILED(dc->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return;
	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->alc = pointLight.alc;
	dataPtr2->ali = pointLight.ali;
	dataPtr2->dlc = pointLight.dlc;
	dataPtr2->dli = pointLight.dli;
	dataPtr2->slc = pointLight.slc;
	dataPtr2->sli = pointLight.sli;
	dataPtr2->pos = pointLight.pos;
	dataPtr2->ePos = Math::fromVec3(camPos, 1.0f);
	dc->Unmap(_lightBuffer, 0);
	dc->PSSetConstantBuffers(0, 1, &_lightBuffer);


	dc->VSSetShader(_vertexShader, NULL, 0);
	dc->PSSetShader(_pixelShader, NULL, 0);
	dc->IASetInputLayout(_layout);
	dc->PSSetSamplers(0, 1, &_sampleState);
}



void TerrainShader::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}