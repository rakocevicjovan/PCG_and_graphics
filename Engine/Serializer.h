#pragma once
#include "SkeletalModel.h"
#include "Model.h"
#include "SerializableAsset.h"



class Serializer
{
public:


	static MemChunk serializeTexture(const Texture& texture)
	{
		// Metadata 4 * 4 bytes, data varies
		int w = texture.getW();
		int h = texture.getH();
		int n = texture.getN();
		TextureRole role = texture._role;

		const unsigned char* data = texture.getData();

		UINT metadataSize = 16;
		UINT dataSize = w * h * n;
		UINT totalSize = metadataSize + dataSize;

		std::unique_ptr<char[]> output(new char[totalSize]);

		// To symmetry... And beyond!
		memcpy(&output.get()[0], &w, sizeof(w));
		memcpy(&output.get()[4], &h, sizeof(n));
		memcpy(&output.get()[8], &n, sizeof(h));
		memcpy(&output.get()[12], &role, sizeof(role));
		memcpy(&output.get()[16], data, dataSize);

		//FileUtils::writeAllBytes(exportPath.c_str(), &output, metadataSize + dataSize);

		return { std::move(output), totalSize };
	}



	static MemChunk serializeAnimation(const Animation& anim)
	{
		anim.getName();
		anim.getTickDuration();
		anim.getNumTicks();
		anim.getDuration();	// Ticks per second can be inferred
	}

};