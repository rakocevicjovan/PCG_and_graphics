#pragma once
#include "Math.h"
#include "Model.h"
#include "ShaderManager.h"
#include "CollisionEngine.h"
#include "Camera.h"

class GraphicComponent
{
public:
	GraphicComponent() {};
	
	template <typename FlexibleShaderType>
	GraphicComponent(Model* m, FlexibleShaderType* s) : model(m), shader(s) {}

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

	SVec3 getPosition()
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

	void setCamera(Camera& camera)
	{
		cam = camera;
	}

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
};