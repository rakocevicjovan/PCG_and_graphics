#pragma once

// System-reserved registers.

constexpr uint32_t PER_CAMERA_CBUFFER_REGISTER = 10u;

constexpr uint32_t PER_FRAME_CBUFFER_REGISTER = 11u;

constexpr uint32_t PS_CSM_CBUFFER_REGISTER = 12u;
constexpr uint32_t PS_CSM_TEXTURE_REGISTER = 11u;


// Structs mirroring layouts of globaly accessible buffers on the gpu.

// Lower frequency - per camera (changes if camera changes projection matrix, for example
struct PerCameraBuffer
{
	SMatrix projection;
	SMatrix invProjection;
	float w;
	float h;
	float n;
	float f;
};

// Higher frequency - per frame
struct VSPerFrameBuffer
{
	SMatrix cameraMatrix;
	SMatrix viewMatrix;
	float delta;
	float elapsed;
	SVec2 padding{};
};

struct PSPerFrameBuffer
{
	SMatrix cameraMatrix;
	SMatrix viewMatrix;
	SVec4 eyePos;
	float delta;
	float elapsed;
	SVec2 padding{};
};