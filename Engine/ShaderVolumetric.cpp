#include "ShaderVolumetric.h"
#include "Camera.h"



ShaderVolumetric::ShaderVolumetric()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_matrixBuffer = nullptr;
	_elapsedTimeBuffer = nullptr;
	_volumShaderBuffer = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
}



ShaderVolumetric::~ShaderVolumetric()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_matrixBuffer)
	DECIMATE(_elapsedTimeBuffer)
	DECIMATE(_volumShaderBuffer)
	DECIMATE(_layout)
	DECIMATE(_sampleState)
}



bool ShaderVolumetric::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	D3D11_BUFFER_DESC elapsedBufferDesc = ShaderCompiler::createBufferDesc(sizeof(ElapsedTimeBuffer));
	result &= shc.createConstantBuffer(elapsedBufferDesc, _elapsedTimeBuffer);

	D3D11_BUFFER_DESC viewRayBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VolumShaderBuffer));
	result &= shc.createConstantBuffer(viewRayBufferDesc, _volumShaderBuffer);

	return result;
}



bool ShaderVolumetric::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& camera, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	VolumShaderBuffer* dataPtr2;
	ElapsedTimeBuffer* dataPtr3;

	SMatrix mT = mMat.Transpose();


	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	if (FAILED(deviceContext->Map(_elapsedTimeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr3 = (ElapsedTimeBuffer*)mappedResource.pData;
	dataPtr3->elapsed = elapsed;
	dataPtr3->padding = SVec3::Zero;
	deviceContext->Unmap(_elapsedTimeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_elapsedTimeBuffer);


	//view data - updates per frame
	if (FAILED(deviceContext->Map(_volumShaderBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (VolumShaderBuffer*)mappedResource.pData;
	dataPtr2->rot = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * 0.5f * elapsed);
	dataPtr2->ePos = Math::fromVec3(camera.GetCameraMatrix().Translation(), 1.0f);
	deviceContext->Unmap(_volumShaderBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &_volumShaderBuffer);


	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void ShaderVolumetric::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}