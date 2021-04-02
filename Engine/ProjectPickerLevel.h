#pragma once
#include "Level.h"
#include "ProjectPicker.h"

class ProjectPickerLevel : public Level
{
	ProjectPicker pp;

public:
	ProjectPickerLevel(Engine& sys) : Level(sys) {}
	
	void init(Engine& sys) override { _sys._inputManager.toggleMouseVisibility(); }

	void update(const RenderContext& rc) override {}

	void ProjectPickerLevel::draw(const RenderContext& rc) override
	{
		rc.d3d->SetBackBufferRenderTarget();

		finished = !pp.Render();

		rc.d3d->EndScene();
	}
};