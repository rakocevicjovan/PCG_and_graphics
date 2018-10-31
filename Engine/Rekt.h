#pragma once

#include <d3d11.h>
#include <vector>
#include "Math.h"
#include "Mesh.h"
#include "ShaderHUD.h"

class Rekt{

public:

	class UINODE {
	public:
		Mesh m;
		SVec2 pos;
		SVec2 size;
		std::vector<UINODE*> children;

		UINODE() {}

		~UINODE() {
			this->Exterminate();
		}
	
		void drawUINODE(ID3D11DeviceContext* deviceContext, ShaderHUD& s, ID3D11ShaderResourceView* srv) {

			s.SetShaderParameters(deviceContext, m);
			deviceContext->PSSetShaderResources(0, 1, &(srv));
			if(m.vertices.size() > 0)
				m.draw(deviceContext, s);
			s.ReleaseShaderParameters(deviceContext);

			for (auto c : children) {				
				c->drawUINODE(deviceContext, s, srv);
			}
		}

		void UINODE::Exterminate() {
			for (auto c : children) {
				(*c).Exterminate();
				delete c;
			}
		}
	};

	Rekt(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~Rekt();

	UINODE* AddUINODE(UINODE* parent, SVec2 pos, SVec2 size);
	void draw(ID3D11DeviceContext* deviceContext, ShaderHUD& s, ID3D11ShaderResourceView* srv);
	UINODE* getRoot() { return &_ROOT; }

private:
	UINODE _ROOT;

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;
};

