#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "CollisionEngine.h"

/*
class Model;
class ShaderBase;
class Camera;
class Collider;
class Controller;
*/

class GraphicComponent
{
public:
	GraphicComponent() {};
	GraphicComponent(Model* m, ShaderBase* s);

	Model* model;
	ShaderBase* shader;
};



class GameObject
{
public:
	GameObject() {};
	~GameObject() {};
};



class Actor : public GameObject
{
public:
	Actor() {};
	Actor(SMatrix& transform, GraphicComponent gc) : transform(transform), gc(gc) {}

	SMatrix transform;
	GraphicComponent gc;
	Collider collider;
};



class Player
{
public:
	Player();
	~Player() {};

	SMatrix transform;
	Collider* collider = nullptr;

	GraphicComponent gfx_comp;
	Controller* con;
	Camera* cam;
};