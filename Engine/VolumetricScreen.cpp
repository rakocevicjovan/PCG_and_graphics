#include "VolumetricScreen.h"
#include "Camera.h"
#include "Mesh.h"

VolumetricScreen::VolumetricScreen() : ShaderBase()
{
}


VolumetricScreen::~VolumetricScreen()
{
	delete screenQuad;
}



bool VolumetricScreen::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	this->filePaths = filePaths;
	if (!ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc))
		return false;

	D3D11_BUFFER_DESC viewRayDesc;
	viewRayDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewRayDesc.ByteWidth = sizeof(VolumetricScreenBuffer);
	viewRayDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewRayDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewRayDesc.MiscFlags = 0;
	viewRayDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&viewRayDesc, NULL, &_viewRayBuffer)))
		return false;

	screenQuad = new Mesh(SVec2(0, 0), SVec2(1, 1), device);

	return true;
}



bool VolumetricScreen::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& camera, const SMatrix& gales, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* matBuffer;
	VolumetricScreenBuffer* volumScreenBuffer;

	//near plane offset
	SMatrix camOffset = camera.GetCameraMatrix();
	Math::Translate(camOffset, camOffset.Backward() * .15f);

	SMatrix mT = camOffset.Transpose();
	SMatrix vT = camera.GetViewMatrix().Transpose();
	SMatrix pT = camera.GetProjectionMatrix().Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	matBuffer = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	matBuffer->world = mT;
	matBuffer->view = vT;
	matBuffer->projection = pT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	
	//view data - updates per frame
	if (FAILED(deviceContext->Map(_viewRayBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	volumScreenBuffer = (VolumetricScreenBuffer*)mappedResource.pData;
	volumScreenBuffer->cameraPosition = Math::fromVec3(camera.GetPosition(), elapsed);
	volumScreenBuffer->cameraDirection = Math::fromVec3(camera.GetCameraMatrix().Backward(), 1.f);
	volumScreenBuffer->gale1 = SVec4(256,	333,	256,	20);
	volumScreenBuffer->gale2 = SVec4(280,	333,	240,	20);
	volumScreenBuffer->gale3 = SVec4(220,	333,	256,	20);
	volumScreenBuffer->gale4 = SVec4(277,	333,	215,	20);
	deviceContext->Unmap(_viewRayBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_viewRayBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}

/*
	volumScreenBuffer->gale1 = SVec4(gales.m[0]);
	volumScreenBuffer->gale2 = SVec4(gales.m[1]);
	volumScreenBuffer->gale3 = SVec4(gales.m[2]);
	volumScreenBuffer->gale4 = SVec4(gales.m[3]);
*/