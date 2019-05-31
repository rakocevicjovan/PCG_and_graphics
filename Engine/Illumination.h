#include "Math.h"

namespace Strife
{

	class Illumination
	{
		//radiative transfer equation in the context of a single bounce... makes use of all the other stuff...
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


		//in scattering at position x from direction w
		//n - number of lights
		//p - phase function
		//v - visibility function
		//l_c_i - direction vector from point to i-th light
		//c_light_i - radiance from i-th light as a function of distance to it 
		float inScattering(SVec3 x, SVec3 w, SVec3 lightPos)
		{
			float p = phaseMieHG(0, 0);			//rayleigh or mie depending on densiteh
			float v = visibility(x, lightPos);	//v = x.isInLight (sample shadow map - [0-1] to approximate soft shadow, otherwise ugly)
			float c_light_i;

			// * SUM[1, n] p(w, l_c_i) * v(x, p_light_i) * c_light_i(distance(x, p_light_i))
			return PI * (p * v * c_light_i);	//bracketed for each light in fact... but I use one only
		}

		//phase functions - dictate scattering probabilities, intuitively described as two lobes around the particle
		//physically based phase functions can depend on the size of the particle scattering the light

		//lambda is the wave length of the light? or something alike to that
		float phasePhysics(float r, float lambda)
		{
			return 2. * PI * r / lambda;
		}

		//naive, "perfect" phase function
		float phaseIsotropic()
		{
			return 1.f / 4.f * PI;	//should be compiler-optimized to return a constant for fast execution
		}

		//sp << 1, Rayleigh scattering
		float phaseRayleigh(float cosTheta)
		{
			//sigS = (0.490, 1.017, 2.339)
			//calculated using sigS(lambda) proportionate to 1.f / (lambda^4);

			return (3. / (16. * PI)) * (1. + cosTheta * cosTheta);
		}

		//sp ~= 1, Mie scattering (Henyey-Greenstein approximation, silver lining effect)
		//The g parameter can be used to represent backward(g < 0), isotropic(g = 0), or forward(g > 0) scattering
		float phaseMieHG(float cosTheta, float g)	// g[-1, 1], also called eccentricity
		{
			float g2 = g * g;

			float numerator = 1.f - g2;

			float toPow = 1.f + g2 - 2.f * g * cosTheta;

			float denominator = 4.f * PI * toPow * sqrt(toPow);	//is pow(x, 3/2) but 
			
			return numerator / denominator;
		}


		//faster because it has no pow, only a square... if g does not change, suitable optimization!
		float phaseMieSchlick(float cosTheta, float k)	//k ~= 1.55g - 0.55g^3
		{
			float arcaneThings = 1.f + k * cosTheta;

			float numerator = 1.f - k * k;
			float denominator = 4.f * PI * arcaneThings * arcaneThings;

			return numerator / denominator;
		}

		//sp >> 1, geometric scattering, not useful within scope
		float phaseGeometric()	//sigmaS isn't wavelength dependent
		{}

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
		SVec3 extinction(float dist, SVec3 sigA)	//, SVec3 initialRadiance
		{
			return -sigA * dist;
		}


		SVec3 BeerLambert(SVec3 sigA, float dist)
		{
			float rx = exp(-sigA.x * dist);
			float ry = exp(-sigA.y * dist);
			float rz = exp(-sigA.z * dist);

			return SVec3(rx, ry, rz);
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
		float inScattering(SVec3 ro, SVec3 rd, SVec3 lightPos, float rayDistance)
		{
			// Calculate coefficients .
			SVec3 lightToRO = ro - lightPos;			//vector from light position to ray origin
			float b = rd.Dot(lightToRO);				//dot product between light to RO and RD (NOT cos angle)
			float sqd_L_RO = lightToRO.Dot(lightToRO);	//squared distance between light and RO
			float s = 1.0f / sqrt(sqd_L_RO - b * b);
			// Factorize some components .
			float x = s * rayDistance;	
			float y = s * b;
			return s * atan((x) / (1.0 + (x + y) * y));
		}
	};

}