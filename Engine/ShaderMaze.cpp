#include "ShaderMaze.h"
#include "Model.h"
#include "Camera.h"

ShaderMaze::ShaderMaze() : ShaderBase()
{
}


ShaderMaze::~ShaderMaze()
{
}

struct MazeVarBuff
{
	SVec4 lul;
};

bool ShaderMaze::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Model& m, const Camera& cam, 
										const PointLight& pLight, float deltaTime, const Texture& diff, const Texture& n)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	LightBuffer* dataPtr2;
	MazeVarBuff* playerPosBuffer;

	SMatrix mT = m.transform.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();


	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
	deviceContext->Unmap(_matrixBuffer, 0);
	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);

	SVec4 pos = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.f);

	if (FAILED(deviceContext->Map(_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	playerPosBuffer = (MazeVarBuff*)mappedResource.pData;
	playerPosBuffer->lul = pos;
	deviceContext->Unmap(_variableBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_variableBuffer);


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
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &_lightBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	
	deviceContext->PSSetSamplers(0, 1, &_sampleState);
	deviceContext->PSSetShaderResources(0, 1, &(diff.srv));
	deviceContext->PSSetShaderResources(1, 1, &(n.srv));

	//deviceContext->VSSetSamplers(0, 1, &_sampleState);
	//deviceContext->VSSetShaderResources(0, 1, &(disp.srv));

	return true;
}



void ShaderMaze::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(unbinder[0]));
	deviceContext->VSSetShaderResources(0, 1, &(unbinder[0]));
}