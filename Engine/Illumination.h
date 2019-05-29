#include "Math.h"

namespace Strife
{

	class Illumination
	{
		//radiative transfer equation in the context of a single bounce
		float calcRadiativeTransfer(SVec3 x, SVec3 v)
		{

		}

		//Medium's albedo - represents the importance of scattering relative to absorption in a medium for 
		//each visible spectrum range considered (the overall reflectiveness of the medium)
		// RO -> 0 means that the medium is murky (exhaust smoke), RO -> 1 means it's reflective (clouds) 
		SVec3 calcMediumAlbedo(SVec3 sigS, SVec3 sigA)
		{
			return sigS / (sigS + sigA);	//sigS / sigT
		}


		//in scattering at position x from direction v
		//n - number of lights
		//p - phase function
		//v - visibility function
		//l_c_i - direction vector from point to i-th light
		//c_light_i - radiance from i-th light as a function of distance to it

		float inscattering(SVec3 x, SVec3 v)
		{
			return PI; // * SUM[1, n] p(v, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i))
		}

		//phase functions - dictate scattering probabilities, intuitively described as two lobes around the particle
		
		//naive, "perfect" phase function
		float phaseIsotropic()
		{
			return 1.f / 4.f * PI;	//should be compiler-optimized to return a constant, implying fast execution
		}
		
		//physically based phase functions depend on the size of the particle scattering the light



		//lambda is the wave length of the light? or something alike to that
		float phasePhysics(float r, float lambda)
		{
			return 2. * PI * r / lambda;
		}

		//sp << 1, Rayleigh scattering
		float phaseRayleigh(float theta)
		{
			//sigS = (0.490, 1.017, 2.339)
			//calculated using sigS(lambda) proportionate to 1.f / (lambda^4);

			float cosTheta = cos(theta);
			return (3. / (16. * PI)) * (1. + cosTheta * cosTheta);
		}

		//sp ~= 1, Mie scattering (Henyey-Greenstein approximation, silver lining effect)
		//The g parameter can be used to represent backward(g < 0), isotropic(g = 0), or forward(g > 0) scattering
		float phaseMieHG(float theta, float g)	// g[-1, 1], also called eccentricity
		{
			float g2 = g * g;

			float numerator = 1.f - g2;
			float denominator = 4.f * PI * pow(1.f + g2 - 2.f * g * cos(theta), 1.5);
			
			//why abs?
			//return ((1.0 - eccentricity * eccentricity) / pow(abs(1.0f + eccentricity * eccentricity - 2.0f * eccentricity * cos_angle), 1.5f)) / 4.0f * PI;
			

			return numerator / denominator;
		}


		//faster because it has no pow, only a square... if g does not change, suitable optimization!
		float phaseMieSchlick(float theta, float k)	//k ~= 1.55g - 0.55g^3
		{
			float arcaneThings = 1.f + k * cos(theta);

			float numerator = 1.f - k * k;
			float denominator = 4.f * PI * arcaneThings * arcaneThings;

			return numerator / denominator;
		}

		//sp >> 1, geometric scattering
		float phaseGeometric()	//sigmaS isn't wavelength dependent
		{

		}

		//visibility function - ratio of light reaching a position x from a light source at lightPos
		//volShad = Tr(x, lightPos)... same thing, which refers to a volumetric shadow, and is in [0, 1] range
		float visibility(SVec3 x, SVec3 lightPos)
		{
			return 0; //shadowMap(x, lightPos) * volShad(x, lightPos);
		}


		//out scattering
		float outScattering(SVec3 x, SVec3 v)
		{
			
		}

		//absorption (Beer Lambert law) - from production volume rendering, Fong et al.
		SVec3 absorption(SVec3 x, SVec3 sigA, SVec3 initialRadiance)
		{
			return -sigA * x * initialRadiance;
		}


		//transmittance
		float transmittance(SVec3 x, SVec3 c)
		{
			float tau; // = [x, c], sigT(x)

			return exp(-tau);
		}


		//emission - likely not going to be used since clouds don't do it


		//equation used to evaluate scattered light from a punctual (point) light source
		//Li(c, -v) = Tr(c, p) * Lo(p, v) + INTEGRATE[0, distance(p, c)]t Tr(c, c - vt) * Lscat(c-vt, v) * SigSDT

		//coastal water transmittance per meter (0.3, 0.73, 0.63), sigT = (1.2, 0.31, 0.46)

		float BeerLambertFog(float density, float zDepth)
		{
			return exp(-density * zDepth);
		}


		//homogenous spherical uniform phase
		float inScattering(SVec3 rayStart, SVec3 rayDir, SVec3 lightPos, float rayDistance)
		{
			// Calculate coefficients .
			SVec3 q = rayStart - lightPos;
			float b = rayDir.Dot(q);
			float c = q.Dot(q);
			float s = 1.0f / sqrt(c - b * b);
			// Factorize some components .
			float x = s * rayDistance;
			float y = s * b;
			return s * atan((x) / (1.0 + (x + y) * y));
		}
	};

}