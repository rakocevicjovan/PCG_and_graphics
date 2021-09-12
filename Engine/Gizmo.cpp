#include "pch.h"

#include "Gizmo.h"
#include "IMGUI/imgui.h"	// Has to be here to forward declare for ImGuizmo.h... bit meh but not really important
#include "ImGuizmo.h"


namespace
{
	inline ImGuizmo::MODE getMode(Gizmo::Space gizmoSpace) { return gizmoSpace == Gizmo::Space::LOCAL ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;  }
	
	constexpr inline ImGuizmo::OPERATION getOp(Gizmo::Op op)
	{
		switch (op)
		{
		case Gizmo::Op::S: return  ImGuizmo::OPERATION::SCALE;
		case Gizmo::Op::R: return  ImGuizmo::OPERATION::ROTATE;
		case Gizmo::Op::T: return  ImGuizmo::OPERATION::TRANSLATE;
		}
		assert(false);
	}
}


void Gizmo::Initialize()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
}


void Gizmo::BeginFrame()
{
	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);
}


float Gizmo::display(SMatrix& target, const SMatrix& view, const SMatrix& proj, Gizmo::Op op, Gizmo::Space gizmoSpace)
{
	ImGuizmo::Enable(_enabled);
	if (_enabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::Manipulate(&view._11, &proj._11, getOp(op), getMode(gizmoSpace), &target._11);
	}
	return 0.f;
}


void Gizmo::enable()
{
	_enabled = true;
}


bool Gizmo::isEnabled()
{
	return _enabled;
}