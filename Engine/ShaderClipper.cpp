#include "ShaderClipper.h"
#include "Model.h"
#include "Camera.h"


ShaderClipper::ShaderClipper()
{
}


ShaderClipper::~ShaderClipper()
{
	DECIMATE(_clipperBuffer)
}



bool ShaderClipper::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc);

	D3D11_BUFFER_DESC clipperBufferDesc;
	clipperBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	clipperBufferDesc.ByteWidth = sizeof(ClipperBuffer);
	clipperBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	clipperBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	clipperBufferDesc.MiscFlags = 0;
	clipperBufferDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&clipperBufferDesc, NULL, &_clipperBuffer)))
		return false;

	return true;
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



bool ShaderClipper::SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& cam, const PointLight& pLight, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	LightBuffer* dataPtr2;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
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

	//if(model.textures_loaded.size() != 0)
	for (int i = 0; i < model.textures_loaded.size(); i++)
		deviceContext->PSSetShaderResources(0, 1, &(model.textures_loaded[i].srv));


	return true;
}



void ShaderClipper::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
}