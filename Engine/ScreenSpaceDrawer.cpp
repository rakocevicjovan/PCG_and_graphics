#include "pch.h"
#include "ScreenSpaceDrawer.h"


ScreenspaceDrawer::ScreenspaceDrawer()
{
	_ROOT.pos = SVec2(0.f, 0.f);
	_ROOT.size = SVec2(1.f, 1.f);
}

ScreenspaceDrawer::~ScreenspaceDrawer() {}



ScreenspaceDrawer::UINODE* ScreenspaceDrawer::AddUINODE(ID3D11Device* device, ScreenspaceDrawer::UINODE* parent, SVec2 pos, SVec2 size, float z)
{
	if (parent == nullptr)
	{
		OutputDebugStringA("You are trying to attach a new UI element to unexisting parent. Master node is _ROOT.");
		exit(8007);
	}

	SVec2 pPos = parent->pos;
	SVec2 pSize = parent->size;

	UINODE* uinode = new UINODE;

	uinode->pos = SVec2(pPos.x + pos.x * pSize.x, pPos.y + pos.y * pSize.y);
	uinode->size = SVec2(pSize.x * size.x, pSize.y * size.y);
	uinode->m = Mesh(uinode->pos, uinode->size, device, z);
	
	parent->children.push_back(uinode);
	
	return uinode;
}



void ScreenspaceDrawer::draw(ID3D11DeviceContext* deviceContext, ShaderHUD& s, ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* bloomSRV)
{
	_ROOT.drawUINODE(deviceContext, s, srv, bloomSRV);
}