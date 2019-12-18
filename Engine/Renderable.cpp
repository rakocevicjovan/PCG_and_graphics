#include "Renderable.h"


//@WARNING BAAAD! this would map/unmap several times... @TODO redo to avoid
void Renderable::updateBuffersAuto(ID3D11DeviceContext* cont) const
{
	for (int i = 0; i < mat->getVS()->_bufferMetaData.size(); i++)
	{
		for (const CBufferFieldDesc& cbfd : mat->getVS()->_bufferMetaData[i]._fields)
		{
			if (cbfd._content == CBUFFER_FIELD_CONTENT::TRANSFORM)
			{
				CBuffer::updateBuffer(cont, mat->getVS()->_cbuffers[i], (UCHAR*)&this->worldTransform.Transpose(), cbfd._size, cbfd._offset);
			}
		}
	}

	for (int i = 0; i < mat->getPS()->_bufferMetaData.size(); i++)
	{
		for (const CBufferFieldDesc& cbfd : mat->getPS()->_bufferMetaData[i]._fields)
		{
			if (cbfd._content == CBUFFER_FIELD_CONTENT::P_LIGHT)
			{
				CBuffer::updateBuffer(cont, mat->getPS()->_cbuffers[i], (UCHAR*)this->pLight, cbfd._size, cbfd._offset);
			}
		}
	}
}