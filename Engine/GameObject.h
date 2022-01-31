#pragma once
#include "Math.h"
#include "Model.h"
#include "Camera.h"
#include "SteeringComponent.h"
#include "Renderable.h"
#include "Collider.h"
#include "SparseSet.h"	// Reconsider this...
#include <memory>

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
	
public:

	SMatrix _transform;
	Collider _collider;
	std::vector<Renderable> _renderables;
	SteeringComponent<Actor> _steerComp;

	Actor* parent{};

	Actor() : _steerComp(this), _collider(this) {};

	Actor(Model* model, SMatrix& transform = SMatrix());
	Actor(const Actor& other);
	virtual ~Actor() {};

	void propagate();

	void addRenderable(const Renderable& renderable, float r);
	void addToRenderQueue(Renderer& renderer, const SVec3& camPos, const SVec3& viewForward);
	void render(const Renderer& r) const;
	

	inline SVec3 getPosition() const { return _transform.Translation(); }

	inline Hull* getBoundingHull(UINT index)
	{
		return _collider.getHull(index);
	}
};


class SimpleActor
{
public:

	SparseSet<Renderable>::Handle handle;
	SMatrix _transform;
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
		SMatrix camMat = cam.getCameraMatrix();
		con.processTransformationTP(dTime, a._transform, camMat);
		cam.setCameraMatrix(camMat);
	}

	inline SVec3 getPosition() { return a._transform.Translation(); }

	void setCamera(Camera& camera) { cam = camera; }
};