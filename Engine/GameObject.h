#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "CollisionEngine.h"
#include "Camera.h"
#include "AiController.h"


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
	Collider* collider;

	SVec3 getPosition() const
	{
		return transform.Translation();
	}

	void Draw(ID3D11DeviceContext* context, Camera& cam, PointLight& pl, float dTime)
	{
		gc.shader->SetShaderParameters(context, transform, cam, pl, dTime);
		gc.model->Draw(context, *gc.shader);
		gc.shader->ReleaseShaderParameters(context);
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

	SVec3 getPosition() { return a.transform.Translation(); }

	void setCamera(Camera& camera) { cam = camera; }
};



class ActorAI
{
	Actor a;
	AiController* brain;
};