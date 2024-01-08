#pragma once

class Mesh;
class Texture;

class BigRenderer
{
public:
	// Holds all rendering data, related to game world via entities
	entt::registry _renderables;


	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::steady_clock _rendering_clock{};
	
	void init()
	{
		_start = _rendering_clock.now();
	}

	// How to avoid unnecessary duplication? Not a job of the renderer, don't submit the same mesh multiple times. Importer and loader should deal with this
	// How to associate game object to meshes 1:N? Does the very common tree structure of meshes even matter to the renderer? Mostly, a game object owns a mesh tree not a single mesh.
	entt::entity create_meshes(Mesh* meshes, size_t size);
	entt::entity create_textures(Texture* textures, size_t size);

	bool create_lights();

	void draw();
};