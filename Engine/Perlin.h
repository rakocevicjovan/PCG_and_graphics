#pragma once
#include "Math.h"
#include "Image.h"

namespace Procedural 
{

	class Perlin
	{
		
	private:

		static std::vector<SVec2> gradients2D; 
		static std::vector<int> hashTable;

		inline static int hash2D(int x, int y);
		inline static float mix(float a, float b, float t);

		std::vector<unsigned char> texture;
		std::vector<float> floatTex;

	public:

		Perlin();
		~Perlin();

		int _w = 0, _h = 0;

		float fade(float t);
		float perlin2d(SVec2 pos);
		void generate2DTexturePerlin(int w, int h, float zoomX, float zoomY);
		float FBM(float amplitude, float frequency, unsigned int octaves, float lacunarity, float gain, SVec2 initialValue);
		void generate2DTextureFBM(int w, int h, float amplitude, float frequency, unsigned int octaves, float lacunarity, float gain, bool warp = false);
		void writeToFile(const char* targetFile);
		Image makeImage();

		void fillFloatVector();
		std::vector<float>& getFloatVector();
		std::vector<unsigned char>& getUCharVector();
	};

}