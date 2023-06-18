#pragma once

#include "MeshDataStructs.h"
#include "AnimChannel.h"


class Bone;


class Animation
{
private:

	std::string _name{};
	std::vector<AnimChannel> _channels;

	double _ticks			{ 0. };
	double _ticksPerSecond	{ 0. };
	double _duration		{ 0. };
	double _tickDuration	{ 0. };

public:

	Animation() noexcept;

	Animation(const std::string& name, double ticks, double ticksPerSecond, int nc) noexcept;

	void getTransformAtTime(const std::vector<Bone>& bones, const SMatrix& glInvT, float elapsed, std::vector<SMatrix>& vec) const;

	void expand();


	uint32_t getAnimChannelIndex(const char* name) const
	{
		auto it = std::find_if(_channels.begin(), _channels.end(), 
			[&name](const AnimChannel& ac)
			{
				return ac._boneName == name;
			});

		return static_cast<uint32_t>((it != _channels.end()) ? std::distance(_channels.begin(), it) : (~0));
	}

	inline void addChannel(const AnimChannel& ac) { _channels.push_back(ac); }

	inline float getNumTicks()			const { return _ticks; };
	inline float getTicksPerSec()		const { return _ticksPerSecond; }
	inline float getTickDuration()		const { return _tickDuration; }
	inline float getDuration()			const { return _duration; }

	inline uint32_t getNumChannels()	const { return _channels.size(); }
	inline const std::vector<AnimChannel>* getChannels() const { return &_channels; }
	inline const AnimChannel* getAnimChannel(uint32_t index) const { return index > _channels.size() ? nullptr : &_channels[index]; }

	inline const std::string& getName()		const { return _name; }

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_name, _channels, _ticks, _ticksPerSecond, _duration, _tickDuration);
	}
};