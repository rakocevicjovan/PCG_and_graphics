#pragma once
#include "Camera.h"
#include "Texture.h"
#include "ShaderCompiler.h"
#include "Renderable.h"
#include "DepthStencil.h"
#include "Viewport.h"


template <UINT numCascades>
struct ShadowBufferData
{
	SMatrix lvpMatrices[numCascades];
	SVec4 cascadeLimits;
};


template <UINT numCascades>
class CSM
{
private:

	uint8_t _nMaps;
	uint16_t _width, _height;

	DepthStencil _depthStencil;
	Viewport _viewport;

	// I trust MJP but not myself. Not sure if I even need this but want to know it's purpose.
	ID3D11DepthStencilView* _readOnlyDSV;

	std::vector<SMatrix> _lvpMats;
	std::vector<float> _distances;
	std::vector<Frustum> _frusta;
	ID3D11InputLayout* _inLay;

	CBuffer _wmBuffer;
	CBuffer _lvpBuffer;
	CBuffer _shadowBuffer;

	ShadowBufferData<numCascades> _shBuffData;

	VertexShader* _vs;
	// Use the pixel shader to deal with transparency shadows, currently only depth stencil buffer.


public:

	bool init(ID3D11Device* device, uint16_t width, uint16_t height, VertexShader* vs)
	{
		_vs = vs;

		// Set the required parameters
		_width = width;
		_height = height;
		_viewport = Viewport(static_cast<float>(_width), static_cast<float>(_height));

		_nMaps = numCascades;

		_lvpMats.resize(numCascades);
		_frusta.resize(numCascades);


		// Initialize buffers used by the csm shader
		_lvpBuffer.init(device, CBuffer::createDesc(sizeof(SMatrix)));
		_wmBuffer.init(device, CBuffer::createDesc(sizeof(SMatrix)));
		_shadowBuffer.init(device, CBuffer::createDesc(sizeof(ShadowBufferData<numCascades>)));

		_depthStencil.createDepthStencil(device, _width, _height, DXGI_FORMAT_D32_FLOAT, 0u, numCascades);

		return true;
	}



	// Expects input frustum corners to be in world space
	SMatrix createLightProjectionMatrix(const std::array<SVec3, 8>& corners, const SMatrix& lvMat, uint8_t vpIndex)
	{
		float minX, maxX;
		float minY, maxY;
		float minZ, maxZ;

		minX = minY = minZ = 999999.f;	// Should use limits instead...
		maxX = maxY = maxZ = -999999.f;

		std::array<SVec3, 8> lViewSpaceCorners;

		// Transform the world space positions of frustum coordinates into light's view space
		for (int i = 0; i < corners.size(); ++i)
		{
			SVec3 pos = SVec3::Transform(corners[i], lvMat);
			lViewSpaceCorners[i] = pos;

			if (pos.x < minX) minX = pos.x;
			if (pos.x > maxX) maxX = pos.x;

			if (pos.y < minY) minY = pos.y;
			if (pos.y > maxY) maxY = pos.y;

			if (pos.z < minZ) minZ = pos.z;
			if (pos.z > maxZ) maxZ = pos.z;
		}

		// @TODO If done like this, only objects in the camera frustum will cast shadows, need to account for it

		return DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
	}



	std::vector<SMatrix> calcProjMats(const Camera& cam, const SMatrix& lightViewMatrix)
	{
		std::vector<SMatrix> projMats;
		projMats.reserve(_nMaps);

		_distances = cam._frustum.calcSplitDistances(_nMaps);
		_shBuffData.cascadeLimits = SVec4(_distances.data());	// Unsafe, maybe?
		std::vector<SMatrix> camFrustumSubdivisionPMs = cam._frustum.createCascadeProjMatrices(_nMaps, _distances);

		for (int i = 0; i < camFrustumSubdivisionPMs.size(); ++i)
		{
			// Obtain the corners in world space
			std::array<SVec3, 8> corners = Frustum::extractCorners(cam.GetViewMatrix() * camFrustumSubdivisionPMs[i]);
			projMats.push_back(createLightProjectionMatrix(corners, lightViewMatrix, i));	// Transform them to light space
			_lvpMats[i] = lightViewMatrix * projMats.back();
			_frusta[i]  = Frustum::createFrustumWithPlanesOnly(_lvpMats[i]);	//Used for collision only, needs only planes, faster
		}

		return projMats;
	}


	// All of this below will probably be refactored to just return it's state and not set it directly...
	void beginShadowPassSequence(ID3D11DeviceContext* context)
	{
		context->VSSetShader(_vs->_vsPtr, nullptr, 0);
		context->PSSetShader(NULL, nullptr, 0);
		_viewport.bind(context);
		_inLay = _vs->_layout;
	}



	void beginShadowPassN(ID3D11DeviceContext* context, uint8_t n)
	{
		_depthStencil.clearView(context, n);
		_depthStencil.bindAsRenderTarget(context, n);

		SMatrix lvpMatTranspose = _lvpMats[n].Transpose();

		_shBuffData.lvpMatrices[n] = lvpMatTranspose;

		_lvpBuffer.updateWithStruct(context, lvpMatTranspose);
		_lvpBuffer.bindToVS(context, 1);
	}



	void drawToCurrentShadowPass(ID3D11DeviceContext* context, Renderable& r)
	{
		//r.updateBuffersAuto(context);
		//r.setBuffers(_deviceContext);

		//this is not flexible, it must use the above somehow in order to work properly @TODO
		SMatrix transformTranspose = r._transform.Transpose();
		_wmBuffer.updateWithStruct(context, transformTranspose);
		_wmBuffer.bindToVS(context, 1);

		context->IASetPrimitiveTopology(r.mat->_primitiveTopology);
		context->IASetInputLayout(_inLay);

		UINT stride = r.mesh->getStride();
		UINT offset = r.mesh->getOffset();

		context->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(r.mesh->_indexBuffer.getIdxCount(), 0, 0);
	}



	void drawToSceneWithCSM(ID3D11DeviceContext* context, Renderable& r)
	{
		context->VSSetShader(r.mat->getVS()->_vsPtr, nullptr, 0);
		context->PSSetShader(r.mat->getPS()->_psPtr, nullptr, 0);

		//r.mat->getPS()->updateBuffersAuto(context, r);
		//r.mat->getPS()->setBuffers(context);

		// Again, bad, needs to be more flexible... but put together the pieces for now
		SMatrix transformTranspose = r._transform.Transpose();
		CBuffer::updateWholeBuffer(context, r.mat->getVS()->_cbuffers[0]._cbPtr, &transformTranspose, sizeof(SMatrix));
		context->VSSetConstantBuffers(0, 1, &(r.mat->getVS()->_cbuffers[0]._cbPtr));

		// Bind usual textures
		r.mat->bindTextures(context);
		r.mat->bindSamplers(context);

		// Bind shadow map array
		_depthStencil.bindAsShaderResource(context, PS_CSM_TEXTURE_REGISTER, 1);

		context->IASetInputLayout(r.mat->getVS()->_layout);
		context->IASetPrimitiveTopology(r.mat->_primitiveTopology);

		UINT stride = r.mesh->getStride();
		UINT offset = r.mesh->getOffset();

		context->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
		context->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

		context->DrawIndexed(r.mesh->_indexBuffer.getIdxCount(), 0, 0);

		unbindTextureArray(context);
	}



	void uploadCSMBuffer(ID3D11DeviceContext* context, UINT slot)
	{
		_shadowBuffer.updateWithStruct(context, _shBuffData);
		_shadowBuffer.bindToPS(context, slot);
	}



	void unbindTextureArray(ID3D11DeviceContext* context)
	{
		ID3D11ShaderResourceView *const pSRV[1] = { NULL };
		context->PSSetShaderResources(PS_CSM_TEXTURE_REGISTER, 1, pSRV);
	}


	void bindShadowMapAsSrv(ID3D11DeviceContext* context, uint8_t index)
	{
		context->PSSetShaderResources(index, 1, _depthStencil.srv());
	}


	inline uint8_t getNMaps() { return _nMaps; }
	inline Frustum& getNthFrustum(uint8_t n) { return _frusta[n]; };
};



/* Can be an useful function but not needed right now
std::vector<Frustum> createShadowPassFrusta(const Camera& cam, const SMatrix& dirLightViewMatrix, const SMatrix dirLightCamMatrix)
{
	std::vector<SMatrix> projMats = calcProjMats(cam, dirLightViewMatrix);

	std::vector<Frustum> frusta;
	frusta.reserve(projMats.size());

	for (int i = 0; i < projMats.size(); ++i)
	{
		//frustumRenderable._transform = projMats[i].Invert() * dirLightCamMatrix;
		//S_RANDY.render(frustumRenderable);

		frusta.emplace_back(projMats[i] * dirLightCamMatrix);
	}

	return frusta;
}
*/