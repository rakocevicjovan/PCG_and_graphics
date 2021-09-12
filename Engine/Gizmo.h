#pragma once


class Gizmo
{
private:

	bool _enabled{ true };
	bool _widget{ true };

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

	float display(SMatrix& target, const SMatrix& view, const SMatrix& proj, Op op, Space gizmoSpace);

	void enable();
	bool isEnabled();
};