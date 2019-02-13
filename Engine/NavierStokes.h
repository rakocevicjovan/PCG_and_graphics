#pragma once
#include <vector>
#include "Math.h"

namespace Procedural
{

	class NavierStokes
	{

		float t;
		std::vector<SVec2> v;
		std::vector<SVec2> w;
		std::vector<SVec2> p;


	public:
		NavierStokes();
		~NavierStokes();

		void eq1() {  };
		void eq2() {  };
		void eq3() {  };

		void advect(SVec2 u);
		void diffuse(SVec2 u);
		void addForces(SVec2 u);
		void computePressure(SVec2 u);

		///nabla 
		//gradient
		//divergence
		//laplacian

		//The Helmholtz-Hodge Decomposition

	};

}