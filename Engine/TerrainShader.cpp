#include "TerrainShader.h"
#include "MeshDataStructs.h"
#include "Camera.h"


TerrainShader::TerrainShader()
{
}



TerrainShader::~TerrainShader()
{
}



bool TerrainShader::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	//ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc);
	
	return true;
}



void TerrainShader::SetShaderParameters(ID3D11DeviceContext* dc, const SMatrix& mt, const Camera& cam, const PointLight& pointLight, float deltaTime)
{
	
	/*
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;

	SMatrix mT = mt.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();

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
	dataPtr2->ePos = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.0f);
	dc->Unmap(_lightBuffer, 0);
	dc->PSSetConstantBuffers(0, 1, &_lightBuffer);

	dc->IASetInputLayout(_layout);
	dc->VSSetShader(_vertexShader, NULL, 0);
	dc->PSSetShader(_pixelShader, NULL, 0);
	dc->PSSetSamplers(0, 1, &_sampleState);
	*/
}