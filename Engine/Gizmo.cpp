#include "pch.h"

#include "Gizmo.h"
#include "IMGUI/imgui.h"	// Has to be here to forward declare for ImGuizmo.h... bit meh but not really important
#include "ImGuizmo.h"


namespace
{
	constexpr static inline ImGuizmo::MODE AsImGuizmoMode(Gizmo::Space gizmoSpace)
	{ 
		return gizmoSpace == Gizmo::Space::LOCAL ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
	}
	
	constexpr static inline ImGuizmo::OPERATION AsImGuizmoOp(Gizmo::Op op)
	{
		switch (op)
		{
		case Gizmo::Op::S: return  ImGuizmo::OPERATION::SCALE;
		case Gizmo::Op::R: return  ImGuizmo::OPERATION::ROTATE;
		case Gizmo::Op::T: return  ImGuizmo::OPERATION::TRANSLATE;
		}
		assert(false);
		return ImGuizmo::OPERATION::TRANSLATE;
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


float Gizmo::display(SMatrix& target, const SMatrix& view, const SMatrix& proj)
{
	if (_enabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::Manipulate(&view._11, &proj._11, AsImGuizmoOp(_op), AsImGuizmoMode(_space), &target._11);
	}
	return 0.f;
}


void Gizmo::setEnabled(bool enabled)
{
	_enabled = enabled;
	ImGuizmo::Enable(_enabled);
}


bool Gizmo::getEnabled()
{
	return _enabled;
}


void Gizmo::setOp(Gizmo::Op op)
{
	_op = op;
}


Gizmo::Op Gizmo::getOp()
{
	return _op;
}


void Gizmo::setSpace(Gizmo::Space space)
{
	_space = space;
}


Gizmo::Space Gizmo::getSpace()
{
	return _space;
}