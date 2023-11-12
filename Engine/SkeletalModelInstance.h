#pragma once
#include "SkeletalModel.h"



class SkeletalModelInstance
{
public:

	SkModel* _skm;
	SMatrix _transform;

	CBuffer _skMatsBuffer;
	std::vector<SMatrix> _skeletonMatrices;

	std::vector<AnimationInstance> _animInstances;

	SkeletalModelInstance() : _skm(nullptr) {}

	bool init(ID3D11Device* device, SkModel* skm)
	{
		_skm = skm;

		for (auto anim : skm->_anims)
			_animInstances.emplace_back(anim);

		uint32_t numBones = _skm->_skeleton->getBoneCount();

		// This should eventually be in a big shared buffer for skeletal animation so no need to think about it too much
		_skMatsBuffer.init(device, CBuffer::createDesc(sizeof(SMatrix) * 200 /*numBones*/));

		_skeletonMatrices.resize(_skm->_skeleton->_bones.size());
		return true;
	}


	// This is bad but temporary
	void update(float dTime, UINT animIndex = 0u)
	{
		for (int i = 0; i < _animInstances.size(); ++i)
		{
			_animInstances[i].update(dTime);
		}

		for (auto& meshNode : _skm->_meshNodeTree)
		{
			//_skm->_meshes[i]._worldSpaceTransform = _skm->_meshNodeTree[i].transform * _transform;
			//@TODO replace this
		}

		if (_animInstances.size() > animIndex)	// Avoid crashing when no anim is loaded
		{
			_animInstances[animIndex].getTransformAtTime(_skm->_skeleton->_bones, _skeletonMatrices, _skm->_skeleton->_globalInverseTransform);
		}
		else
		{
			for (SMatrix& mat : _skeletonMatrices)
				mat = SMatrix::Identity;
		}

		for (auto i = 0u; i < _skm->_skeleton->_numInfluenceBones; ++i)
		{
			_skeletonMatrices[i] = _skeletonMatrices[i].Transpose();
		}
	}


	void draw(ID3D11DeviceContext* context)
	{
		_skMatsBuffer.update(context, _skeletonMatrices.data(), sizeof(SMatrix) * _skm->_skeleton->_numInfluenceBones);
		_skMatsBuffer.bindToVS(context, 1);

		for (auto& meshNode : _skm->_meshNodeTree)
		{
			SMatrix meshNodeTf = (_transform * meshNode.transform).Transpose();

			for (auto& meshIndex : meshNode.meshes)
			{
				auto& mesh = _skm->_meshes[meshIndex];
				mesh._material->getVS()->updateCBufferDirectly(context, &meshNodeTf, 0);
				mesh.draw(context);
			}
		}
	}
};