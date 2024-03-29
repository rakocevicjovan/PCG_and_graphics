#include "pch.h"
#include "BitMapper.h"



	BitMapper::BitMapper(Texture& tex) : _texture(tex)
	{
	}


	void BitMapper::init(int smoothness, float widthScale, float lengthScale, float heightScale)
	{
		granularity = smoothness;
		xScale = widthScale;
		yScale = heightScale;
		zScale = lengthScale;
	}


	bool isGrayscale(std::vector<int>& colours)
	{
		const int c0 = colours[0];

		for (int z = 1; z < colours.size(); z++)
		{
			if (colours[z] != c0)
				return false;
		}

		return true;
	}


	float toGray(std::vector<int>& colours)
	{
		float total = std::accumulate(colours.begin(), colours.end(), 0.0f);
		return total / (float)colours.size();
	}


	bool BitMapper::createTerrain()
	{
		// @TODO Make this into an image when its split fully
		//if (!_t.loadFromStoredPath())
			//return false;

		assert(false);

		if (_texture.w() % 2 != 0 || _texture.h() % 2 != 0) {
			std::cout << "Bitmap's width and height have to be even numbers." << std::endl;
			return false;
		}

		int size = _texture.w() * _texture.h() * _texture.nc();

		std::vector<int> colours;
		colours.reserve(_texture.nc());

		inVertMap.resize(_texture.h());
		for (auto vec : inVertMap)
		{
			vec.reserve(_texture.w());
		}

		int x = 0;
		int y = 0;
		float z;

		//go through the data set filled with groups of rgba (number of channels could vary of course) per pixel and conver them to std::vectors
		for (int i = 0; i < size; i += _texture.nc())
		{
			//fill up the colours std::vector
			for (int j = 0; j < _texture.nc(); j++)
				colours.push_back(_texture.getData()[i + j]);

			//check if the pixel is grayscale, if not, normalize the colours and construct the position std::vector for the vertex
			if (isGrayscale(colours))
				z = static_cast<float>(colours[0]);
			else
				z = toGray(colours);

			SVec3 currentVertex = SVec3(static_cast<float>(xScale * x), static_cast<float>(yScale* z) , static_cast<float>(zScale  * y));

			//indices are one indexed so I will increment it here, not that it really matters but just to keep it consistent
			++x;
			inVertMap[y].push_back(std::make_pair(y*_texture.w() + x, currentVertex));

			//Update everything for the next pixel->vertex conversion
			colours.clear();

			if (x == _texture.w())
			{
				x = 0;
				++y;
			}

			// this should honestly never happen...
			if (y == _texture.h())
				break;
		}

		//fill face vectors, calculate face normals
		faces.resize(_texture.h() - 1);
		for (auto fRow : faces)
			fRow.reserve((_texture.w() - 1) * 2);
		
		int faceRow = 0;
		int columnCounter = 0;

		for (int row = 0; row < inVertMap.size() - 1; row++) {

			std::vector<std::pair<int, SVec3>> currentRow = inVertMap[row];
			std::vector<std::pair<int, SVec3>> nextRow = inVertMap[row + 1];

			for (int k = 0; k < inVertMap[row].size() - 1; k++) {		

				SVec3 a, b, c, ab, ac;
				a = currentRow[k + 1].second;
				b = currentRow[k].second;
				c = nextRow[k].second;

				ab = b - a;
				ac = c - a;

				faces[faceRow].push_back(std::make_pair(
					SVec3(
						static_cast<float>(currentRow[k + 1].first),
						static_cast<float>(currentRow[k].first),
						static_cast<float>(nextRow[k].first)),
					ab.SVec3::Cross(ac)));

				//second face
				a = currentRow[k + 1].second;
				b = nextRow[k].second;
				c = nextRow[k + 1].second;

				ab = b - a;
				ac = c - a;

				faces[faceRow].push_back(std::make_pair(
					SVec3(
						static_cast<float>(currentRow[k + 1].first),
						static_cast<float>(nextRow[k].first),
						static_cast<float>(nextRow[k + 1].first)),
					ab.SVec3::Cross(ac)));

				columnCounter++;

				if (columnCounter == _texture.w()) {
					faceRow++;
					columnCounter = 0;
				}

			}
		}

		//calculate vertex normals from face normals
		vertexNormals.reserve(_texture.h() * _texture.w());

		int vertexRow = 0;

		for (auto row : inVertMap)
		{
			std::vector<std::pair<SVec3, SVec3>> pRow;
			std::vector<std::pair<SVec3, SVec3>> nRow;
			
			if (vertexRow == 0) 
			{
				nRow = faces[vertexRow];
			} 
			else if (vertexRow == _texture.h() - 1)
			{
				pRow = faces[vertexRow - 1];
			} 
			else 
			{
				pRow = faces[vertexRow - 1];
				nRow = faces[vertexRow];
			}

			pRow.reserve(pRow.capacity() + nRow.capacity());	
			pRow.insert(pRow.end(), nRow.begin(), nRow.end());

			for (auto inverted : row)
			{
				int thisIndex = inverted.first;
				SVec3 thisNormal = SVec3(0, 0, 0);

				int facesFound = 0;

				for (const auto& face : pRow)
				{
					if (thisIndex == face.first.x || thisIndex == face.first.y || thisIndex == face.first.z)
					{
						thisNormal += face.second;
						++facesFound;
					}

					if (facesFound == 6)	//be careful if you change the structure of the triangles, for now this is good
						break;
				}

				if (!(fabs(thisNormal.Length()) < 0.0001f))
				{
					thisNormal.Normalize();
				}

				vertexNormals.push_back(std::make_pair(thisIndex, thisNormal));
			}

			vertexRow++;
		}

		return true;
	}


	bool BitMapper::terrainToFile(const std::string& filePath)
	{
		outputFileName = filePath;

		std::ofstream outputFile;

		outputFile.open(outputFileName.c_str(), std::ios::out);
		outputFile.precision(6);
		outputFile.setf(std::ios::fixed, std::ios::floatfield);
		
		outputFile << "#Model generated by BTT tool by Jovan Rakocevic" << "\n\n";

		//mtllib path_to_mtl.mtl

		outputFile << "g default \n";

		for (auto row : inVertMap)
		{
			for (auto inVert : row)
			{
				SVec3 pos = inVert.second;
				outputFile << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
			}
			
		}

		for (auto vn : vertexNormals)
		{
			outputFile << "vn " << vn.second.x << " " << vn.second.y << " " << vn.second.z << "\n";
		}

		outputFile << "s off \n";	//possibly unnecessary if I have vertex normals

		outputFile << "g terrain \n";

		//set the material here if you must with usemtl name_of_mtl

		for (auto faceRow : faces)
		{
			for (auto face : faceRow)
			{
				auto& [x, y, z] = face.first;
				// Not sure what was this for...
				//outputFile << "f " << x << "//" << x << " " << y << "//" << y << " " << z << "//" << z << std::endl;
				outputFile << "f " << x << " " << y << " " << z << "\n";
			}
		}

		return true;
	}