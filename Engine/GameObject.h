#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "SteeringComponent.h"
#include "Renderable.h"

class Collider;
class Renderer;

class GameObject
{
public:
	GameObject() {};
	~GameObject() {};
};



//template <typename FlexibleShaderType>	not sure how to do this really... as then actor has to be templated as well
//which seems ugly... or store a base pointer in there? seems there needs to be inheritance somewhere regardless with this approach
class GraphicComponent
{
public:
	GraphicComponent() {};
	
	GraphicComponent(Model* m, ShaderLight* s) : model(std::move(m)), shader(s) {}

	Model* model;
	ShaderLight* shader;
};



class Actor : public GameObject
{
public:
	Actor() : _steerComp(this) {};
	Actor(SMatrix& transform, GraphicComponent gc) : transform(transform), gc(gc), _steerComp(this) {}
	Actor(Model* model, SMatrix& transform = SMatrix());

	Actor* parent;

	SMatrix transform;
	GraphicComponent gc;
	std::vector<Renderable> renderables;
	Collider* _collider;
	SteeringComponent<Actor> _steerComp;

	inline SVec3 getPosition() const
	{
		return transform.Translation();
	}


	//lets just pretend im not a monkey and have a way to get some approximate radius for steering separation...
	float getPersonalDistance() { return 2.f; }	

	void propagate();

	void Draw(ID3D11DeviceContext* context, Camera& cam, PointLight& pl, float dTime)
	{
		gc.shader->SetShaderParameters(context, transform, cam, pl, dTime);
		gc.model->Draw(context, *gc.shader);
		gc.shader->ReleaseShaderParameters(context);
	}

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

	void Draw(ID3D11DeviceContext* context, PointLight& pl, float dTime)
	{
		a.Draw(context, cam, pl, dTime);
	}

	void UpdateCamTP(float dTime)
	{
		SMatrix camMat = cam.GetCameraMatrix();
		con.processTransformationTP(dTime, a.transform, camMat);
		cam.SetCameraMatrix(camMat);
	}

	inline SVec3 getPosition() { return a.transform.Translation(); }

	void setCamera(Camera& camera) { cam = camera; }
};



/*
class ActorAI : public Actor
{
	Actor a;
	AiController* brain;
};
*/