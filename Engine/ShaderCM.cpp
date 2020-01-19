#include "ShaderCM.h"
#include "Model.h"
#include "Camera.h"



ShaderCM::ShaderCM()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_lightBuffer = nullptr;
}



ShaderCM::~ShaderCM()
{
	ShutdownShader();
}



bool ShaderCM::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	_filePaths = filePaths;

	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	//@TODO Check if this is just the default
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.MipLODBias = 0.0f;
	//samplerDesc.MaxAnisotropy = 1;
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//samplerDesc.MinLOD = 0;
	//samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


	D3D11_BUFFER_DESC matrixBufferDesc = shc.createBufferDesc(sizeof(WMBuffer));
	shc.createConstantBuffer(matrixBufferDesc, _matrixBuffer);

	D3D11_BUFFER_DESC lightBufferDesc = shc.createBufferDesc(sizeof(LightBuffer));
	shc.createConstantBuffer(lightBufferDesc, _lightBuffer);

	return true;
}



void ShaderCM::ShutdownShader()
{
	DECIMATE(_lightBuffer);
	DECIMATE(_variableBuffer);
	DECIMATE(_matrixBuffer);
	DECIMATE(_sampleState);
	DECIMATE(_layout);
	DECIMATE(_pixelShader);
	DECIMATE(_vertexShader);
}



bool ShaderCM::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& dLight, float deltaTime, ID3D11ShaderResourceView* tex)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;


	SMatrix mT = mMat.Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->alc = dLight.alc;
	dataPtr2->ali = dLight.ali;
	dataPtr2->dlc = dLight.dlc;
	dataPtr2->dli = dLight.dli;
	dataPtr2->slc = dLight.slc;
	dataPtr2->sli = dLight.sli;
	dataPtr2->pos = dLight.pos;	//actually direction but uses the same struct...
	SVec4 uwotm8 = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.f);
	dataPtr2->ePos = uwotm8;
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);


	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);
	deviceContext->PSSetShaderResources(0, 1, &(tex));

	return true;
}



bool ShaderCM::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	return true;
}