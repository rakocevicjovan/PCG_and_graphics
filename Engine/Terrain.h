#pragma once
#include <d3d11_4.h>
#include <vector>
#include "MeshDataStructs.h"
#include "Math.h"
#include "Light.h"
#include "Texture.h"

class Phong;
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
		std::vector<Vert3D> _vertices;
		std::vector<unsigned int> indices;
		std::vector<std::vector<TangentTriface>> faces;
		std::vector<Texture> textures;

		unsigned int _numRows, _numColumns;
		SVec2 _texCoordScale;
		SVec3 _offset, _scale;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

		ID3D11ShaderResourceView* unbinder[1] = { nullptr };

		///helper functions
		inline unsigned int wr(int row) { return row < 0 ? _numRows + row : row % _numRows; }
		inline unsigned int wc(int col) { return col < 0 ? _numColumns + col : col % _numColumns; }
		float sampleDiamond(int i, int j, int reach);

	public:
		
		Terrain() {}
		Terrain(unsigned int x, unsigned int y, SVec3 scale = SVec3(1, 1, 1), SVec3 offset = SVec3(0, 0, 0));
		~Terrain();

		/// Generation methods
		// Diamond square
		void GenWithDS(SVec4 corners, unsigned int steps, float decay, float randomMax);

		// Load from heightmap
		void GenFromTexture(unsigned int width, unsigned int height, const std::vector<float>& data);

		///manipulation methods
		void Tumble(float chance, float displacement);
		void CellularAutomata(float initialDistribtuion, unsigned int steps);
		void Fault(const SRay& line, float displacement);
		void NoisyFault(const SRay& line, float vertDp, float horiDp, float perlinZoom);
		void TerraSlash(const SRay& line, float displacement, unsigned int steps, float decay);
		void CircleOfScorn(const SVec2& center, float radius, float angle, float displacement, unsigned int steps, float initAngle = 0.f);
		void Mesa(const SVec2& center, float radius, float bandWidth, float height);
		void Smooth(unsigned int steps);	

		/// Wrapping up and directX integration
		void CalculateNormals();
		void CalculateTexCoords();
		SVec3 calculateTangent(const std::vector<Vert3D>& vertices, UINT i0, UINT i1, UINT i2);
		bool SetUp(ID3D11Device* device);
		void Draw(ID3D11DeviceContext* dc, Phong& s, const Camera& cam, const PointLight& pointLight, float deltaTime);

		void populateMesh(std::vector<uint8_t>& verts, std::vector<unsigned int>& inds) const
		{
			verts.resize(_vertices.size() * sizeof(Vert3D));
			memcpy(verts.data(), _vertices.data(), verts.size());
			inds = indices;
		}

		//utility for movement
		float getHeightAtPosition(const SVec3& playerPos);
		
		//getters and setters
		unsigned int	getNumCols() const { return _numColumns; }
		unsigned int	getNumRows() const { return _numRows;    }
		auto&			getVerts()   const { return _vertices;   }
		SVec3			getOffset()  const { return _offset;     }
		std::vector<SVec2> getHorizontalPositions();

		void setOffset(float x, float y, float z) { _offset = SVec3(x, y, z); }
		void setOffset(const SVec3& offset) { _offset = offset; }
		void setScale(float x = 1.f, float y = 1.f, float z = 1.f) { _scale = SVec3(x, y, z); }
		void setScale(const SVec3& scale) { _scale = scale; }
		void setTextureData(ID3D11Device* device, float xRepeat, float zRepeat, std::vector<std::string> textureNames);
	
		inline UINT addToFace(UINT index, const TangentTriface& face, SVec3& normal, SVec3& tangent)
		{
			if (index == face.x || index == face.y || index == face.z)
			{
				normal += face.normal;
				tangent += face.tangent;
				return 1;
			}
			return 0;
		}
	};
}