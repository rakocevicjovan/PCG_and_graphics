#pragma once


class Gizmo
{
public:

	enum class Space : uint8_t
	{
		LOCAL = 0,
		WORLD = 1
	};

	enum class Op : uint8_t
	{
		S = 0,
		R = 1,
		T = 2
	};


	// Internal use, don't call on your own (unless you are future me). Has no EndFrame equivalent, probably wraps up along with ImGui?
	static void Initialize();

	// Internal use, don't call on your own (unless you are future me). Has no EndFrame equivalent, probably wraps up along with ImGui?
	static void BeginFrame();

	float display(SMatrix& target, const SMatrix& view, const SMatrix& proj);

	void setEnabled(bool enabled);
	bool getEnabled();

	void setOp(Gizmo::Op op);
	Gizmo::Op getOp();

	void setSpace(Gizmo::Space space);
	Gizmo::Space getSpace();

private:

	bool _enabled{ true };
	bool _widget{ true };

	Gizmo::Op _op{Gizmo::Op::T};
	Gizmo::Space _space{Gizmo::Space::WORLD};
};