#pragma once

#include <d3d11.h>
#include <vector>
#include "Math.h"
#include "Mesh.h"
#include "ShaderHUD.h"

class ScreenspaceDrawer
{

public:

	class UINODE {
	public:
		Mesh m;
		SVec2 pos;
		SVec2 size;
		std::vector<UINODE*> children;

		UINODE() {}

		~UINODE() { this->Exterminate(); }
	

		template <typename FlexibleShaderType>
		void drawUINODE(ID3D11DeviceContext* deviceContext, FlexibleShaderType& s, ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* bloomSRV = nullptr)
		{
			s.SetShaderParameters(deviceContext, m);
			deviceContext->PSSetShaderResources(0, 1, &srv);
			
			if (bloomSRV)
				deviceContext->PSSetShaderResources(1, 1, &bloomSRV);

			if(m.vertices.size() > 0)
				m.draw(deviceContext, s);

			s.ReleaseShaderParameters(deviceContext);

			for (auto c : children)
				c->drawUINODE(deviceContext, s, srv, bloomSRV);
		}



		void UINODE::Exterminate() 
		{
			for (UINODE* c : children)
			{
				c->Exterminate();
				delete c;
			}
		}
	};

	ScreenspaceDrawer();
	~ScreenspaceDrawer();

	UINODE* AddUINODE(ID3D11Device* device, UINODE* parent, SVec2 pos, SVec2 size, float z = 0.f);
	void draw(ID3D11DeviceContext* deviceContext, ShaderHUD& s, ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* bloomSRV = nullptr);
	UINODE* getRoot() { return &_ROOT; }

private:
	UINODE _ROOT;
};

