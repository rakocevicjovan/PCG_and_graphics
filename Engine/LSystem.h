#pragma once
#include <vector>
#include <map>
#include "Math.h"
#include "MeshDataStructs.h"
#include "Phong.h"
#include "Model.h"

namespace Procedural
{

	struct RewriteRule 
	{
		RewriteRule(char left, std::string right) : l(left), r(right) {};

		bool operator==(const RewriteRule& other) const
		{
			return (l == other.l && r == other.r);
		}

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

		Model tree;

	public:
		LSystem() : _axiom("F"), _current("F") {};
		LSystem(std::string axiom);
		~LSystem();

		void reseed(std::string axiom);
		bool addRule(char left, std::string right);
		void removeRule(char left, std::string right);
		void rewrite(unsigned int steps);
		void genVerts(float length, float decay, float pitch, float yaw);
		Model genModel(ID3D11Device* pDevice, float length, float radius, const float lengthConstriction, const float radiusConstriction, float pitch, float yaw);
		Model genFlower(ID3D11Device* pDevice, Model* petalModel, float stalkSegmentLength, float stalkRadius, float deescalator, float angle, float tilt);
		void setUp(ID3D11Device* pDevice);
		void drawAsLines(ID3D11DeviceContext* dc, Phong& s,
			const SMatrix& mt, const SMatrix& vt, const SMatrix& pt,
			const PointLight& dLight, float deltaTime, SVec3 eyePos);
		std::string getString() { return _current; }
	};

}