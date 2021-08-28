#pragma once


template <typename FrameData>
struct KeyFrame
{
	FrameData frameData{};
	float tick{ 0.f };

	KeyFrame() = default;

	KeyFrame(FrameData frameData, float tick)
		: frameData(frameData), tick(tick)
	{}

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(frameData, tick);
	}
};