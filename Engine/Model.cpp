#include "pch.h"
#include "Model.h"
#include "CollisionEngine.h"
#include "Terrain.h"
#include "FileUtilities.h"
#include "Collider.h"
#include "MatImporter.h"

Model::Model(const Procedural::Terrain& terrain, ID3D11Device* device)
{
	_meshes.emplace_back(terrain, device);
	_transform = SMatrix::CreateTranslation(terrain.getOffset());
}