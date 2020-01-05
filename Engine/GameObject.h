#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "SteeringComponent.h"
#include "Renderable.h"
#include <memory>

class Collider;
class Renderer;

class GameObject
{
public:
	GameObject() {};
	~GameObject() {};
};



class Actor : public GameObject
{
public:
	Actor() : _steerComp(this) {};
	Actor(Model* model, SMatrix& transform = SMatrix());
	virtual ~Actor()
	{
		/*delete _collider;*/
	};

	Actor* parent;

	SMatrix transform;
	std::vector<Renderable> renderables;
	Collider* _collider;				//make it a unique ptr later, safer...
	SteeringComponent<Actor> _steerComp;

	inline SVec3 getPosition() const { return transform.Translation(); }

	//lets just pretend im not a monkey and have a way to get some approximate radius for steering separation...
	float getPersonalDistance() { return 2.f; }	

	void propagate();

	void render(const Renderer& r) const;
};



class Player
{
public:

	Actor a;
	Controller& con;
	Camera cam;

	Player(Controller& c) : con(c)
	{
		cam._controller = &con;
	};

	~Player() {};

	void UpdateCamTP(float dTime)
	{
		SMatrix camMat = cam.GetCameraMatrix();
		con.processTransformationTP(dTime, a.transform, camMat);
		cam.SetCameraMatrix(camMat);
	}

	inline SVec3 getPosition() { return a.transform.Translation(); }

	void setCamera(Camera& camera) { cam = camera; }
};