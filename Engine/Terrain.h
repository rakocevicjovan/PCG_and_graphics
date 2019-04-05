#pragma once
#include <d3d11.h>
#include <vector>
#include "MeshDataStructs.h"
#include "Math.h"
#include "Light.h"
#include "Texture.h"

class ShaderBase;
class Camera;

namespace Procedural 
{
	//helper structs

	struct CACell
	{
		CACell() {}
		CACell(Vert3D vert, bool doa) : vertex(vert), deadOrAlive(doa) {}

		Vert3D vertex;
		bool deadOrAlive;
	};

	struct Triface
	{
		Triface() {}
		Triface(UINT a, UINT b, UINT c) : x(a), y(b), z(c) {}
		Triface(UINT a, UINT b, UINT c, SVec3 n) : x(a), y(b), z(c), normal(n) {}
		
		UINT x = 0, y = 0, z = 0;
		SVec3 normal;
	};

	struct TangentTriface : Triface
	{
		TangentTriface() {}
		TangentTriface(UINT a, UINT b, UINT c) : Triface(a, b, c) {}
		TangentTriface(UINT a, UINT b, UINT c, const SVec3& n, const SVec3& t) : Triface(a, b, c, n), tangent(t) {}

		SVec3 tangent;
	};



	class Terrain
	{
		std::vector<Vert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<std::vector<TangentTriface>> faces;
		std::vector<Texture> textures;

		unsigned int _numRows, _numColumns;
		float xScale = 1.0f, yScale = 1.0f, zScale = 1.0f;
		float tcxr = 1.f, tczr = 1.f;
		SVec3 _offset;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

		ID3D11ShaderResourceView* unbinder[1] = { nullptr };

		///helper functions
		inline unsigned int wr(int row);
		inline unsigned int wc(int col);
		float sampleDiamond(int i, int j, int reach);

	public:
		
		Terrain(unsigned int x = 1, unsigned int y = 1, SVec3 scales = SVec3(1, 1, 1));
		~Terrain();

		void setScales(float x, float y, float z);
		void setTextureData(ID3D11Device* device, float xRepeat, float zRepeat, std::vector<std::string> textureNames);

		///generation methods

		//diamond square (midpoint displacement for 2D)
		void GenWithDS(SVec4 corners, unsigned int steps, float decay, float randomMax);

		//load from heightmap
		void GenFromTexture(unsigned int width, unsigned int height, const std::vector<float>& data);

		///manipulation methods
		void Tumble(float chance);
		void CellularAutomata(float initialDistribtuion, unsigned int steps);
		void Fault(const SRay& line, float displacement);
		void NoisyFault(const SRay& line, float vertDp, float horiDp, float perlinZoom);
		void TerraSlash(const SRay& line, float displacement, unsigned int steps, float decay);
		void CircleOfScorn(const SVec2& center, float radius, float angle, float displacement, unsigned int steps, float initAngle = 0.f);
		void Mesa(const SVec2& center, float radius, float bandWidth, float height);
		void Smooth(unsigned int steps);	

		
		///wrapping up and directX integration
		void CalculateNormals();
		void CalculateTexCoords();
		SVec3 calculateTangent(const std::vector<Vert3D>& vertices, UINT i0, UINT i1, UINT i2);
		bool SetUp(ID3D11Device* device);
		void Draw(ID3D11DeviceContext* dc, ShaderBase& s, const Camera& cam, const PointLight& pointLight, float deltaTime);


		///getters
		unsigned int	getNumCols()	{ return _numColumns;	}
		unsigned int	getNumRows()	{ return _numRows;		}
		auto&			getVerts()		{ return vertices;		}


		SVec3 getOffset() const	{ return _offset; }
		void setOffset(float x, float y, float z) { _offset = SVec3(x, y, z); }

		void populateMesh(std::vector<Vert3D>& verts, std::vector<unsigned int>& inds, std::vector<Texture>& tex) const
		{ 
			verts = vertices;
			inds = indices;
			tex = textures;
		}
		
		///stuff
		std::vector<SVec2> getHorizontalPositions();
		float getHeightAtPosition(const SVec3& playerPos);
	};
}

/*

private:
	bool CalculateNormals();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);



*/