#include "SkelAnimShader.h"
#include "SkeletalModel.h"
#include "Camera.h"
#include "ShaderDataStructs.h"

ShaderSkelAnim::ShaderSkelAnim()
{
	_vertexShader = 0;
	_pixelShader = 0;
	_layout = 0;
	_sampleState = 0;
	_matrixBuffer = 0;
	_lightBuffer = 0;
}


ShaderSkelAnim::~ShaderSkelAnim()
{
}


bool ShaderSkelAnim::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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

	D3D11_BUFFER_DESC boneBufferDesc = ShaderCompiler::createBufferDesc(sizeof(SMatrix) * MAX_BONES);
	shc.createConstantBuffer(boneBufferDesc, _bonesBuffer);

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);


	for (int i = 0; i < MAX_BONES; ++i)
		boneTransformBuffer.boneTransforms.emplace_back(SMatrix::Identity);

	return result;
}



void ShaderSkelAnim::ShutdownShader()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_matrixBuffer)
	DECIMATE(_bonesBuffer)
	DECIMATE(_lightBuffer)
	DECIMATE(_layout)
	DECIMATE(_sampleState)
}



bool ShaderSkelAnim::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SVec3& camPos, 
	const PointLight& pLight, const std::vector<SMatrix>& boneTransformsIn)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	LightBuffer* dataPtr2;
	BoneTransformBuffer* dataPtr4;

	SMatrix mT = mMat.Transpose();

	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);


	result = deviceContext->Map(_bonesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr4 = (BoneTransformBuffer*)mappedResource.pData;
	memcpy(mappedResource.pData, boneTransformsIn.data(), boneTransformsIn.size() * sizeof(SMatrix));
	deviceContext->Unmap(_bonesBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_bonesBuffer);


	result = deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr2 = (LightBuffer*)mappedResource.pData;
	dataPtr2->alc = pLight.alc;
	dataPtr2->ali = pLight.ali;
	dataPtr2->dlc = pLight.dlc;
	dataPtr2->dli = pLight.dli;
	dataPtr2->slc = pLight.slc;
	dataPtr2->sli = pLight.sli;
	dataPtr2->pos = pLight.pos;
	dataPtr2->ePos = Math::fromVec3(camPos, 1.0f);
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);


	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void ShaderSkelAnim::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}