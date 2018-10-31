#include "Rekt.h"

Rekt::Rekt(ID3D11Device* device, ID3D11DeviceContext* deviceContext){
	
	_device = device;
	_deviceContext = deviceContext;

	_ROOT.pos = SVec2(0.f, 0.f);
	_ROOT.size = SVec2(1.f, 1.f);
}



Rekt::~Rekt(){
}



Rekt::UINODE* Rekt::AddUINODE(Rekt::UINODE* parent, SVec2 pos, SVec2 size){
	
	if (parent == nullptr) {
		OutputDebugStringA("You are trying to attach a new UI element to unexisting parent. Master node is _ROOT.");
		exit(8007);
	}

	SVec2 pPos = parent->pos;	// parent == nullptr ? SVec2(0.f, 0.f) :
	SVec2 pSize = parent->size;	//parent == nullptr ? SVec2(1.f, 1.f) :

	UINODE* uinode = new UINODE;

	uinode->pos = SVec2(pPos.x + pos.x * pSize.x, pPos.y + pos.y * pSize.y);
	uinode->size = SVec2(pSize.x * size.x, pSize.y * size.y);
	uinode->m = Mesh(uinode->pos, uinode->size, _device);
	
	parent->children.push_back(uinode);
	
	return uinode;
}

/*void Rekt::draw(ID3D11DeviceContext* deviceContext, ShaderHUD& s) {
	_ROOT.drawUINODE(deviceContext, s);
}*/

void Rekt::draw(ID3D11DeviceContext* deviceContext, ShaderHUD& s, ID3D11ShaderResourceView* srv) {
	_ROOT.drawUINODE(deviceContext, s, srv);
}