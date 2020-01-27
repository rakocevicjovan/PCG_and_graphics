#include "Phong.h"
#include "Camera.h"



Phong::Phong()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_sampleState = nullptr;
	_matrixBuffer = nullptr;
	_lightBuffer = nullptr;
}



Phong::~Phong()
{
	DECIMATE(_vertexShader);
	DECIMATE(_pixelShader);
	DECIMATE(_layout);
	DECIMATE(_sampleState);
	DECIMATE(_matrixBuffer);
	DECIMATE(_lightBuffer);
}



bool Phong::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
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



bool Phong::SetShaderParameters(ID3D11DeviceContext* dc, SMatrix& modelMat, const Camera& cam, const PointLight& pLight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	WMBuffer* wmptr;
	LightBuffer* lptr;

	SMatrix mT = modelMat.Transpose();

	//WMBuffer
	if (FAILED(dc->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	wmptr = (WMBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	wmptr->world = mT;
	dc->Unmap(_matrixBuffer, 0);

	bufferNumber = 0;
	dc->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);
	//


	//LightBuffer
	if (FAILED(dc->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	lptr = (LightBuffer*)mappedResource.pData;

	lptr->alc = pLight.alc;
	lptr->ali = pLight.ali;
	lptr->dlc = pLight.dlc;
	lptr->dli = pLight.dli;
	lptr->slc = pLight.slc;
	lptr->sli = pLight.sli;
	lptr->pos = pLight.pos;
	lptr->ePos = Math::fromVec3(cam.GetPosition(), 1.f);

	dc->Unmap(_lightBuffer, 0);

	bufferNumber = 0;
	dc->PSSetConstantBuffers(bufferNumber, 1, &_lightBuffer);
	//


	//generalize this for all materials
	dc->IASetInputLayout(_layout);
	dc->VSSetShader(_vertexShader, NULL, 0);
	dc->PSSetShader(_pixelShader, NULL, 0);
	dc->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void Phong::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}