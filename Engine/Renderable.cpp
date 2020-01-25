#include "Renderable.h"


//Reworked to avoid mapping multiple times which could happen using the following in the loop:
//CBuffer::updateBuffer(cont, curBuffer, &this->worldTransform.Transpose(), cbfd._size, cbfd._offset);
void Renderable::updateBuffersAuto(ID3D11DeviceContext* cont) const
{
	D3D11_MAPPED_SUBRESOURCE mr;

	for (int i = 0; i < mat->getVS()->_bufferMetaData.size(); i++)
	{
		ID3D11Buffer* curBuffer = mat->getVS()->_cbuffers[i];
		CBuffer::map(cont, curBuffer, mr);

		for (const CBufferFieldDesc& cbfd : mat->getVS()->_bufferMetaData[i]._fields)
		{
			if (cbfd._content == CBUFFER_FIELD_CONTENT::TRANSFORM)
			{
				CBuffer::updateField(curBuffer, &worldTransform.Transpose(), cbfd._size, cbfd._offset, mr);
			}
		}

		CBuffer::unmap(cont, curBuffer);
	}

	for (int i = 0; i < mat->getPS()->_bufferMetaData.size(); i++)
	{
		ID3D11Buffer* curBuffer = mat->getPS()->_cbuffers[i];
		CBuffer::map(cont, curBuffer, mr);

		for (const CBufferFieldDesc& cbfd : mat->getPS()->_bufferMetaData[i]._fields)
		{
			if (cbfd._content == CBUFFER_FIELD_CONTENT::P_LIGHT)
			{
				CBuffer::updateField(curBuffer, pLight, cbfd._size, cbfd._offset, mr);
			}
		}

		CBuffer::unmap(cont, curBuffer);
	}
}