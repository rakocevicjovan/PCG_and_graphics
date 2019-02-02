#pragma once
#include <d3d11.h>
#include <vector>
#include "MeshDataStructs.h"
#include "Math.h"
#include "lightclass.h"
#include "Shader.h"


namespace Procedural 
{

	struct MatrixBufferType 
	{
		SMatrix world;
		SMatrix view;
		SMatrix projection;
	};

	struct VariableBufferType 
	{
		float deltaTime;
		SVec3 padding;	//what a fucking waste of bandwidth gg microsoft
	};

	struct LightBufferType 
	{
		SVec3 alc;
		float ali;
		SVec3  dlc;
		float dli;
		SVec3 slc;
		float sli;
		SVec4 pos;
		SVec4 ePos;
	};



	class CACell
	{
	public:
		CACell() {}
		CACell(Vert3D vert, bool doa) : vertex(vert), deadOrAlive(doa) {}

		Vert3D vertex;
		bool deadOrAlive;
	};



	class Terrain
	{

	private:

		std::vector<Vert3D> vertices;
		std::vector<unsigned int> indices;
		std::vector<std::vector<Vert3D>> verticesDoubleVector;
		unsigned int _numRows, _numColumns;
		float xScale = 10.0f, yScale = 10.0f, zScale = 10.0f;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

		ID3D11ShaderResourceView* unbinder[1] = { nullptr };

		//helper functions
		inline unsigned int wr(int row);
		inline unsigned int wc(int col);

		float sampleDiamond(int i, int j, int reach);

	public:
		
		Terrain(unsigned int x = 1, unsigned int y = 1);
		~Terrain();

		void setScales(float x, float y, float z);

		///height generation methods

		//random
		void GenRandom(float chance);

		//diamond square (midpoint displacement for 2D)
		void GenWithDS(SVec4 corners, unsigned int steps, float decay, float randomMax);

		//cellular automata
		void GenWithCA(float initialDistribtuion, unsigned int steps);

		//load from heightmap
		void GenFromTexture(unsigned int width, unsigned int height, const std::vector<float>& data);


		///manipulation methods

		//faulting - using z = k * x + offset
		void fault(const SRay& line, float displacement);
		void faultIterative(const SRay& line, float displacement, unsigned int steps, float decay);

		//y[i] = k * y[i-j] + (1-k) * x[i], where k is a filtering constant (erosion coefficient) such that 0 <= k <= 1
		//apply this FIR function to rows and columns individually, in both directions
		void faultFilter();	


		///wrapping up and directX integration
		void CalculateNormals();
		bool SetUp(ID3D11Device* device);
		void Draw(ID3D11DeviceContext* dc, Shader& s, 
			const SMatrix& mt, const SMatrix& vt, const SMatrix& pt, 
			const PointLight& dLight, float deltaTime, SVec3 eyePos);
	};
}

/*

private:
	bool CalculateNormals();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);



*/