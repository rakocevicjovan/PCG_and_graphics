#include "pch.h"

#include "Level.h"


Level::Level(Engine& sys) : _sys(sys) {}


#pragma region Audio
/*
musicLSystem.reseed("d");
musicLSystem.addRule('d', "Fa");
musicLSystem.addRule('e', "Gb");
musicLSystem.addRule('f', "Ac");
musicLSystem.addRule('g', "bD");
musicLSystem.addRule('a', "cE");
musicLSystem.addRule('b', "dF");
musicLSystem.addRule('c', "Eg");

musicLSystem.addRule('D', "aD");
musicLSystem.addRule('E', "gB");
musicLSystem.addRule('F', "aC");
musicLSystem.addRule('G', "Bd");
musicLSystem.addRule('A', "Ce");
musicLSystem.addRule('B', "Df");
musicLSystem.addRule('C', "eG");

musicLSystem.rewrite(6);

std::string lSystemNotes = musicLSystem.getString();
std::vector<std::string> notes;
for (char lsn : lSystemNotes)
	notes.push_back(std::string(1, lsn));

audio.init();
audio.storeSequence(notes);
*/
#pragma endregion Audio


#pragma region ProceduralTesting
/*
//proceduralTerrain.GenWithDS(SVec4(0.f, 10.f, 20.f, 30.f), 4u, 0.6f, 10.f);

///Cellular automata testing
//proceduralTerrain.CellularAutomata(0.5f, 0);

///Noise testing	-SVec3(4, 100, 4) scaling with these fbm settings looks great for perlin
//perlin.generate2DTexturePerlin(512, 512, 16.f, 16.f);	//
//perlin.generate2DTextureFBM(256, 256, 1, 1., 4u, 2.1039f, .517f, true);	//(256, 256, 1.f, 1.f, 3, 2.f, .5f);
//proceduralTerrain.GenFromTexture(perlin._w, perlin._h, perlin.getFloatVector());
//perlin.writeToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png");



///Ridge/turbluent noise testing - looks quite nice actually
//Texture tempTex;
//auto fltVec = tempTex.generateTurbulent(256, 256, 1.f, 1.61803, 0.5793f, 6u);
//auto fltVec = tempTex.generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
//Texture::WriteToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png", tempTex.w, tempTex.h, 1, tempTex.data, 0);
//proceduralTerrain.GenFromTexture(tempTex.w, tempTex.h, fltVec);


///Terrain deformation testng
//proceduralTerrain.fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
//proceduralTerrain.Fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
//proceduralTerrain.NoisyFault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), -20.f);
//proceduralTerrain.NoisyFault(SRay(SVec3(75.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), +15.f);
//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
//proceduralTerrain.CircleOfScorn(SVec2(proceduralTerrain.getNumCols() / 2, proceduralTerrain.getNumRows() / 2), 40.f, PI * 0.01337f, .5f, 64);
//proceduralTerrain.Smooth(3);

///Voronoi tests - shatter stores an array of the same size as the vertPositions vector, each member containing the index of the closest seed
//Procedural::Voronoi v;
//v.init(25, proceduralTerrain.getNumCols(), proceduralTerrain.getNumRows());
//std::vector<SVec2> vertPositions = proceduralTerrain.getHorizontalPositions();
//v.shatter(vertPositions);

///L-systems testing
linden.reseed("F");
linden.addRule('F', "FF+[+F-F-F]*-[-F+F+F]/"); //"[-F]*F[+F][/F]"	//"F[+F]F[-F]+F" for planar		//"FF+[+F-F-F]*-[-F+F+F]/"
//linden.addRule('F', "F[+F]F[-F]+F");

//linden.reseed("F+F+F+F");
//linden.addRule('F', "FF+F-F+F+FF");

linden.rewrite(4);

float liangle = PI * 0.138888f;		//liangle = PI * .5f;

flowerModel = linden.genModel(device, 6.99f, 1.f, .7f, .7f, liangle, liangle);

//Math::RotateMatByMat(flowerModel.transform, SMatrix::CreateRotationX(-PI * .5f));
//linden.genVerts(20.f, 0.8f, PI * 0.16666f, PI * 0.16666f);	linden.setUp(_device);
*/
#pragma endregion