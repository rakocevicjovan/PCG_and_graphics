#include "GameObject.h"
#include "ShaderManager.h"
#include "Model.h"
#include "CollisionEngine.h"



GraphicComponent::GraphicComponent(Model* m, ShaderBase* s) : model(m), shader(s) {}



Player::Player() {}

Player::~Player() {}