#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Light.h"
#include "MeshDataStructs.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

struct RoleTexturePair
{
	TextureRole _role;
	Texture* _tex;
};

//contains everything that the attached shaders need to be set to run... I guess?
class Material
{
protected:

	//most important sorting criteria
	VertexShader* _vertexShader;
	PixelShader* _pixelShader;

public:

	//second most important sorting criteria
	std::vector<RoleTexturePair> _texDescription;
	
	// determines whether it goes to the transparent or opaque queue
	bool _opaque;

	//this could also belong in the vertex buffer... like stride and offset do
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Functions
	Material();
	Material(VertexShader* vs, PixelShader* ps, bool opaque);
	~Material();

	void bindTextures(ID3D11DeviceContext* context);
	
	inline VertexShader* getVS() const { return _vertexShader; }
	inline PixelShader* getPS() const { return _pixelShader; }

	void setVS(VertexShader* vs);
	void setPS(PixelShader* ps);

	template <typename Archive>
	void serialize(Archive& ar, std::vector<UINT>& texIDs)
	{
		ar(0u, 0u, texIDs);
	}
};