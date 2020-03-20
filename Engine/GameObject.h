#pragma once
#include "Math.h"
#include "Model.h"
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

	Actor* parent;

	SMatrix transform;
	std::vector<Renderable> _renderables;
	Collider _collider;
	SteeringComponent<Actor> _steerComp;
	std::vector<PLight*> _pLights;
	std::vector<SLight*> _sLights;



	Actor() : _steerComp(this) {};
	Actor(Model* model, SMatrix& transform = SMatrix());
	Actor(const Actor& other);
	virtual ~Actor() {};

	void patchMaterial(VertexShader* vs, PixelShader* ps, PointLight& pLight);
	void addRenderable(const Renderable& renderable);
	void propagate();
	void render(const Renderer& r) const;

	inline SVec3 getPosition() const { return transform.Translation(); }

	inline Hull* getBoundingHull(UINT index)
	{
		return _collider.getHull(index);
	}
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