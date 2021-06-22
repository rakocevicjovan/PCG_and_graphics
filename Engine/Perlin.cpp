#include "pch.h"
#include "Perlin.h"
#include "Chaos.h"
#include "Texture.h"


namespace Procedural
{

	Perlin::Perlin()
	{
		// Populating with random permutation
		hashTable.resize(512);
		std::iota(hashTable.begin(), hashTable.begin() + 256, 0);

		Chaos c;
		float seed = c.rollTheDice();
		std::default_random_engine engine(seed);
		std::shuffle(hashTable.begin(), hashTable.begin() + 256, engine);
		std::copy_n(hashTable.begin(), 256, hashTable.begin() + 256);
	}



	Perlin::~Perlin()
	{
	}



	float Perlin::fade(float t)
	{
		return pow(t, 3) * (t * (6 * t - 15) + 10);
	}



	inline int Perlin::hash2D(int x, int y)
	{
		return hashTable[hashTable[x] + y];
	}



	inline float Perlin::mix(float a, float b, float t)
	{
		return (1 - t)*a + t * b;
	}

	

	#define FASTFLOOR(x) ( (x >= 0.f) ? ((int)x) : ((int)x-1 ) )
	float Perlin::perlin2d(SVec2 pos)
	{

		//determine the square this point belongs to
		int iLeft = FASTFLOOR(pos.x);
		int iBottom = FASTFLOOR(pos.y);

		//get fracts and fade them
		float xf = pos.x - iLeft;
		float yf = pos.y - iBottom;
		float u = fade(xf);
		float v = fade(yf);
		
		//wrap
		iLeft = iLeft < 0 ? 255 + iLeft : iLeft % 255;
		iBottom = iBottom < 0 ? 255 + iBottom : iBottom % 255;

		//define the corners of the square
		SVec2 bottomLeft(iLeft, iBottom);
		SVec2 bottomRight(iLeft + 1.0f, iBottom);
		SVec2 topLeft(iLeft, iBottom + 1.0f);
		SVec2 topRight(iLeft + 1.0f, iBottom + 1.0f);

		//find gradients of the corners from the precomputed array
		SVec2 gradBL = gradients2D[hash2D(iLeft, iBottom) % 8];
		SVec2 gradBR = gradients2D[hash2D(iLeft + 1, iBottom) % 8];
		SVec2 gradTL = gradients2D[hash2D(iLeft, iBottom + 1) % 8];
		SVec2 gradTR = gradients2D[hash2D(iLeft + 1, iBottom + 1) % 8];

		//calculate vectors from the corners to the position vector...
		SVec2 deltaBL(xf, yf);
		SVec2 deltaBR(xf - 1.f, yf);
		SVec2 deltaTL(xf, yf - 1.f);
		SVec2 deltaTR(xf - 1.f, yf - 1.f);

		//calculate dot products of the delta vectors with the gradient vectors
		float dotBL = deltaBL.Dot(gradBL);
		float dotBR = deltaBR.Dot(gradBR);
		float dotTL = deltaTL.Dot(gradTL);
		float dotTR = deltaTR.Dot(gradTR);

		//interpolate between results
		float result = mix(mix(dotBL, dotBR, u), mix(dotTL, dotTR, u), v);
		
		//if(result < 1.01f || result > -1.01f)
		assert(result < 1.01f && result > -1.01f);
		
		return result;
	}



	void Perlin::generate2DTexturePerlin(int w, int h, float zoomX, float zoomY)
	{
		_w = w;
		_h = h;

		texture.reserve(w * h);
		floatTex.reserve(w * h);

		float wInverse = 1.f / (float)w;
		float hInverse = 1.f / (float)h;

		for (int i = 0; i < w; ++i)
		{
			for (int j = 0; j < h; ++j)
			{
				//how far along the image we are, normalized to 0-1 range...
				float x = (float)i * wInverse;
				float y = (float)j * hInverse;

				float rgb = perlin2d(SVec2(x * zoomX, y * zoomY));
				int r = (int)((rgb + 1.f) * 0.5f * 255.f);
				unsigned char uc = (unsigned char)r;

				texture.push_back(uc);
				floatTex.push_back(rgb);
			}
		}
	}



	//amplitude[~1], frequency[~1], gain[0, 1], lacunarity[1+] are recommended for the defaultish look
	//amplitude and frequency are the intial values, and each octave amplitude is multiplied by gain and frequency by lacunarity
	float Perlin::FBM(float amplitude, float frequency, unsigned int octaves, float lacunarity, float gain, SVec2 initialValue)
	{
		float accumulated = 0.f;

		for (int i = 0; i < octaves; ++i)
		{
			accumulated += Perlin::perlin2d(frequency * initialValue) * amplitude;
			frequency *= lacunarity;
			amplitude *= gain;
		}
		return accumulated;
	}



	void Perlin::generate2DTextureFBM(int w, int h, float amplitude, float frequency, unsigned int octaves, float lacunarity, float gain, bool warp) 
	{
		_w = w;
		_h = h;

		texture.reserve(w * h);
		floatTex.reserve(w * h);

		float wInverse = 1.f / (float)w;
		float hInverse = 1.f / (float)h;

		for (int i = 0; i < w; ++i)
		{
			for (int j = 0; j < h; ++j)
			{
				//how far along the image we are, normalized to 0-1 range...
				float x = (float)i * wInverse;
				float y = (float)j * hInverse;

				SVec2 curPos(x, y);

				//spatial domain warping... nice results, but no idea how to predict what they will look like at all
				if (warp)
				{
					float tf = FBM(amplitude, frequency, octaves, lacunarity, gain, curPos);
					curPos = SVec2(x + tf, y + tf);
				}
					

				float rgb = FBM(amplitude, frequency, octaves, lacunarity, gain, curPos);
				int r = (int)((rgb + 1.f) * 0.5f * 255.f);
				unsigned char uc(r);
				texture.push_back(uc);
				floatTex.push_back(rgb);
			}
		}
	}



	void Perlin::writeToFile(const char* targetFile)
	{
		assert(false);	// Change for image if needed in the future, not worth thinking about rn
		//Texture::SaveAsPng(targetFile, _w, _h, 1, texture.data(), 0);	// sizeof(unsigned char) * w
	}



	void Perlin::fillFloatVector()
	{
		floatTex.reserve(texture.size());
		for (int i = 0; i < texture.size(); i++)
		{
			floatTex.push_back((float)texture[i]);
		}
	}


	std::vector<float>& Perlin::getFloatVector() 
	{
		return floatTex;
	}

	std::vector<unsigned char>& Perlin::getUCharVector()
	{
		return texture;
	}



	std::vector<SVec2> Perlin::gradients2D =
	{
	SVec2(0.707f, 0.707f),
	SVec2(0.707f, -0.707f),
	SVec2(-0.707f, 0.707f),
	SVec2(-0.707f, -0.707f),
	SVec2(0.f, 1.f),
	SVec2(0.f, -1.f),
	SVec2(1.f, 0.f),
	SVec2(-1.f, 0.f)
	};



	std::vector<int> Perlin::hashTable =
	{
		151, 160, 137, 91, 90, 15,
		131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
		190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
		88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
		77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
		102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
		135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
		5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
		223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
		129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
		251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
		49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
	};
}