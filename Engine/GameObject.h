#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "SteeringComponent.h"
#include "Renderable.h"
#include <memory>
#include "Collider.h"

class Renderer;

class GameObject
{
public:
	GameObject() {};
	~GameObject() {};
};



class Actor : public GameObject
{
private:
	//void copyShenanigans(const Actor& other);
	
public:
	Actor() : _steerComp(this) {};
	Actor(Model* model, SMatrix& transform = SMatrix());
	Actor(const Actor& other);
	virtual ~Actor();
	
	//void operator=(const Actor& other);	I think this is ok as a shallow assignment by default...
	//virtual Actor* Clone() const;

	void patchMaterial(VertexShader* vs, PixelShader* ps, PointLight& pLight);

	Actor* parent;

	SMatrix transform;
	std::vector<Renderable> renderables;
	Collider _collider;				//make it a unique ptr later, safer...
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