#include "VolumetricScreen.h"
#include "Camera.h"
#include "Mesh.h"

VolumetricScreen::VolumetricScreen()
{
}


VolumetricScreen::~VolumetricScreen()
{
	DECIMATE(_vertexShader);
	DECIMATE(_pixelShader);
	DECIMATE(_matrixBuffer);
	DECIMATE(_volumScreenBuffer);
	DECIMATE(_layout);
	DECIMATE(_sampleState);

	delete screenQuad;
}



bool VolumetricScreen::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC volumScreenBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VolumetricScreenBuffer));
	result &= shc.createConstantBuffer(volumScreenBufferDesc, _volumScreenBuffer);

	screenQuad = new Mesh(SVec2(0, 0), SVec2(1, 1), shc.getDevice());

	return result;
}



bool VolumetricScreen::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& camera, const SMatrix& gales, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* matBuffer;
	VolumetricScreenBuffer* volumScreenBuffer;

	//near plane offset
	SMatrix camOffset = camera.GetCameraMatrix();
	Math::Translate(camOffset, camOffset.Backward() * .15f);

	SMatrix mT = camOffset.Transpose();
	
	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	matBuffer = (WMBuffer*)mappedResource.pData;
	matBuffer->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	

	if (FAILED(deviceContext->Map(_volumScreenBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	volumScreenBuffer = (VolumetricScreenBuffer*)mappedResource.pData;
	volumScreenBuffer->cameraPosition = Math::fromVec3(camera.GetPosition(), elapsed);
	volumScreenBuffer->cameraDirection = Math::fromVec3(camera.GetCameraMatrix().Backward(), 1.f);
	volumScreenBuffer->gale1 = SVec4(256,	333,	256,	20);
	volumScreenBuffer->gale2 = SVec4(280,	333,	240,	20);
	volumScreenBuffer->gale3 = SVec4(220,	333,	256,	20);
	volumScreenBuffer->gale4 = SVec4(277,	333,	215,	20);
	deviceContext->Unmap(_volumScreenBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_volumScreenBuffer);


	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void VolumetricScreen::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}