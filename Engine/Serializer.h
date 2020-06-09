#pragma once
#include "SkeletalModel.h"
#include "Model.h"

// I'd wrap it in a class or namespace but cba typing it... I know right.

struct MemChunk
{
	std::unique_ptr<char> ptr;
	UINT size;
};

class Serializer
{
public:
	// Serialization functions for asset classes
	static MemChunk serializeSkeletalModel(const SkeletalModel& skm)
	{
		MemChunk result;

		std::vector<MemChunk> memChunks;

		memChunks.reserve(skm._meshes.size());

		for (int i = 0; i < skm._meshes.size(); ++i)
			memChunks.push_back(serializeSkeletalMesh(skm._meshes[i]));


		//FileUtils::writeAllBytes(_exportPath.c_str(), memChunks[i].ptr.get(), memChunks[i].size);
		return result;
	}



	static MemChunk serializeSkeletalMesh(const SkeletalMesh& mesh)
	{
		// Header data
		UINT indexCount = mesh._indices.size();
		UINT vertexCount = mesh._vertices.size();
		//UINT texCount = mesh._textures.size();

		UINT ibs = indexCount * sizeof(UINT);
		UINT vbs = vertexCount * sizeof(BonedVert3D);
		//UINT tbs = texCount * sizeof(UINT);	// These will be indices... not sure how that's gonna work

		UINT headerSize = 12;
		UINT dataSize = ibs + vbs;		// + tbs

		UINT totalSize = headerSize + dataSize;

		std::unique_ptr<char> result(new char(totalSize));

		UINT offset = 0u;	// Wrap in a helper if this works

		memcpy(result.get() + offset, &indexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result.get() + offset, &vertexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result.get() + offset, &vertexCount, sizeof(UINT));
		offset += sizeof(UINT);

		memcpy(result.get() + offset, mesh._indices.data(), ibs);
		offset += ibs;

		memcpy(result.get() + offset, mesh._vertices.data(), vbs);
		offset += vbs;

		return { std::move(result), totalSize };
	}



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

		std::unique_ptr<char> output(new char(totalSize));

		// To symmetry... And beyond!
		memcpy(&output.get()[0], &w, sizeof(w));
		memcpy(&output.get()[4], &h, sizeof(n));
		memcpy(&output.get()[8], &n, sizeof(h));
		memcpy(&output.get()[12], &role, sizeof(role));
		memcpy(&output.get()[16], data, dataSize);

		//FileUtils::writeAllBytes(exportPath.c_str(), &output, metadataSize + dataSize);

		return { std::move(output), totalSize };
	}

};