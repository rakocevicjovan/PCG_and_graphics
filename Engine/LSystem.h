#pragma once
#include <vector>
#include <map>
#include "Math.h"
#include "MeshDataStructs.h"
#include "ShaderLight.h"

namespace Procedural
{

	struct MatrixBufferTypeA
	{
		SMatrix world;
		SMatrix view;
		SMatrix projection;
	};
	struct VariableBufferTypeA
	{
		float deltaTime;
		SVec3 padding;	//what a fucking waste of bandwidth gg microsoft
	};
	struct LightBufferTypeA
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



	struct RewriteRule 
	{
		RewriteRule(char left, std::string right) : l(left), r(right) {};

		char l;
		std::string r;
	};



	class LSystem
	{
	private:

		std::string _current, _next;
		std::vector<RewriteRule> _rules;
		std::string _axiom;

		std::vector<Vert3D> verts;
		std::vector<unsigned int> indices;
		ID3D11Buffer *_vertexBuffer, *_indexBuffer;

	public:
		LSystem() : _axiom("f"), _current("f") {};
		LSystem(std::string axiom);
		~LSystem();

		void reseed(std::string axiom);
		bool addRule(char left, std::string right);
		void rewrite(unsigned int steps);
		void genVerts(float length, float decay, float pitch, float yaw);
		void setUp(ID3D11Device* device);
		void draw(ID3D11DeviceContext* dc, ShaderLight& s,
			const SMatrix& mt, const SMatrix& vt, const SMatrix& pt,
			const PointLight& dLight, float deltaTime, SVec3 eyePos);
	};

}