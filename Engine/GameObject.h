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


//can be a bunch of packed bits but do I really need it to be? this class can produce an int64_t eventually
class RenderableQueueKey
{
public:
	unsigned char renderTarget;		//max 256 render targets...
	int16_t materialId;				//max 2^16 (65536) materials
	int16_t textureId;				//max 2^16 (65536) texture combinations within material...sort this out
	unsigned char vertexFormat;		//max 256 vertex formats

	int64_t create64bitKey()
	{
		//leaves 16 (64 - 48) bits free for whatever other stuff I might sort by, can be further compressed too
		int64_t result = renderTarget << (63-8) | materialId << (63-24) | textureId << (63-40) | vertexFormat << (63 - 48);
	}
};



class Renderable
{
public:
	Mesh* mesh;
	Material* mat;

	//dynamic? not sure
	int64_t sortKey;
	float zDepth;

	Renderable(Mesh* m, Material* mat) : mesh(m), mat(mat) {}

	//overload after deciding how to sort them
	bool Renderable::operator < (const Renderable& b) const
	{
		return sortKey < b.sortKey;
	}
	
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

	Actor(SMatrix& transform, Model& m) : transform(transform)
	{
		renderables.reserve(m.meshes.size());
		for (Mesh m : m.meshes)
		{
			Renderable(&m, m.material);
		}
	}

	SMatrix transform;
	GraphicComponent gc;
	std::vector<Renderable> renderables;
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