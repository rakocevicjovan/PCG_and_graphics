#include "pch.h"

#include "RendererSystem.h"

#include <immintrin.h>


// TODO move to some utility header
void print__mm128(__m128 var)
{
	float val[4];
	memcpy(val, &var, sizeof(val));

	char str[256];
	sprintf_s(str, sizeof(str), "Numerical: %f %f %f %f \n",
		val[0], val[1], val[2], val[3]);
	OutputDebugStringA(str);
}


void RendererSystem::frustumCull(const Camera& cam)
{
	// Test code, using this projection matrix and identity view matrix before calling the function
	//testCam.setProjectionMatrix(DirectX::XMMatrixOrthographicLH(1024, 1024, 0, 1000));
	//hulls.size = 2;
	//hulls.x = { -509, 500 };
	//hulls.y = { 8, 400 };
	//hulls.z = { 5, 992 };
	//hulls.r = { 4, 10 };

	//std::vector<int> results;
	//results.resize(hulls.size);

	// Handle special case when 0 real hulls exist.
	if (_registry->view<SphereHull>().size() < 2)
	{
		return;
	}

	assure_valid();


	const auto& pl = cam._frustum._planes;

	// NB: _mm_set_ps stores the parameters in right-to-left order, this way the results are easier to debug, can also use _mm_setr_ps
	__m128 px_0123 = _mm_set_ps(pl[3].x, pl[2].x, pl[1].x, pl[0].x);
	__m128 py_0123 = _mm_set_ps(pl[3].y, pl[2].y, pl[1].y, pl[0].y);
	__m128 pz_0123 = _mm_set_ps(pl[3].z, pl[2].z, pl[1].z, pl[0].z);
	__m128 pd_0123 = _mm_set_ps(pl[3].w, pl[2].w, pl[1].w, pl[0].w);

	__m128 px_4545 = _mm_set_ps(pl[5].x, pl[4].x, pl[5].x, pl[4].x);
	__m128 py_4545 = _mm_set_ps(pl[5].y, pl[4].y, pl[5].y, pl[4].y);
	__m128 pz_4545 = _mm_set_ps(pl[5].z, pl[4].z, pl[5].z, pl[4].z);
	__m128 pd_4545 = _mm_set_ps(pl[5].w, pl[4].w, pl[5].w, pl[4].w);

	auto hulls = _registry->raw<SphereHull>();
	auto numHulls = _registry->size<SphereHull>();

	auto hullView = _registry->view<SphereHull>();
	auto begin = hullView.begin();
	auto end = hullView.end();

	for (auto iter = begin; iter < end - 1; iter += 2)
	{
		const auto& hull1 = hullView.get<SphereHull>(*iter);
		const auto& hull2 = hullView.get<SphereHull>(*(iter + 1));

		// First sphere against first 4 planes
		__m128 ax4 = _mm_load_ps1(&hull1.v.x);
		__m128 ay4 = _mm_load_ps1(&hull1.v.y);
		__m128 az4 = _mm_load_ps1(&hull1.v.z);
		__m128 ar4 = _mm_load_ps1(&hull1.v.w);

		__m128 dot_a_0123 = _mm_fmadd_ps(az4, pz_0123, pd_0123);
		dot_a_0123 = _mm_fmadd_ps(ay4, py_0123, dot_a_0123);
		dot_a_0123 = _mm_fmadd_ps(ax4, px_0123, dot_a_0123);

		dot_a_0123 = _mm_cmpge_ps(_mm_add_ps(dot_a_0123, ar4), __m128{0, 0, 0, 0});

		// Second spheree against first 4 planes
		__m128 bx4 = _mm_load_ps1(&hull2.v.x);
		__m128 by4 = _mm_load_ps1(&hull2.v.y);
		__m128 bz4 = _mm_load_ps1(&hull2.v.z);
		__m128 br4 = _mm_load_ps1(&hull2.v.w);

		__m128 dot_b_0123 = _mm_fmadd_ps(bz4, pz_0123, pd_0123);
		dot_b_0123 = _mm_fmadd_ps(by4, py_0123, dot_b_0123);
		dot_b_0123 = _mm_fmadd_ps(bx4, px_0123, dot_b_0123);

		dot_b_0123 = _mm_cmpge_ps(_mm_add_ps(dot_b_0123, br4), __m128{0, 0, 0, 0});

		// Positions combined, against the remaining 2 planes
		__m128 abx4 = _mm_shuffle_ps(ax4, bx4, _MM_SHUFFLE(0, 1, 0, 1));
		__m128 aby4 = _mm_shuffle_ps(ay4, by4, _MM_SHUFFLE(0, 1, 0, 1));
		__m128 abz4 = _mm_shuffle_ps(az4, bz4, _MM_SHUFFLE(0, 1, 0, 1));
		__m128 abr4 = _mm_shuffle_ps(ar4, br4, _MM_SHUFFLE(0, 1, 0, 1));

		__m128 dot_ab_4545 = _mm_fmadd_ps(abz4, pz_4545, pd_4545);
		dot_ab_4545 = _mm_fmadd_ps(aby4, py_4545, dot_ab_4545);
		dot_ab_4545 = _mm_fmadd_ps(abx4, px_4545, dot_ab_4545);

		dot_ab_4545 = _mm_cmpge_ps(_mm_add_ps(dot_ab_4545, abr4), __m128{0, 0, 0, 0});

		// NB: A float with all exponent bits set to 1 is shown by VS debugger as a NaN (in line with IEEE specification)
		// Since _mm_cmpge_ps sets all the bits when true, it produces "-nan" - this is not an error!
		// Also, the bit mask returned by _mm_movemask_ps is in reverse where (0, -nan, -nan, -nan) becomes 1110
		int res_a3210 = _mm_movemask_ps(dot_a_0123);
		int res_b3210 = _mm_movemask_ps(dot_b_0123);
		int res_b54a54 = _mm_movemask_ps(dot_ab_4545);

		// Mask the two ls bits for a, and the following 2 for b, final results is a or b is in plane 543210
		int res_a = res_a3210 + ((res_b54a54 & 0b0011) << 4);
		int res_b = res_b3210 + ((res_b54a54 & 0b1100) << 2);

		_registry->get<VisibleFlag>(*iter).val = (res_a == 0b111111);
		_registry->get<VisibleFlag>(*(iter + 1)).val = (res_b == 0b111111);
	}
}