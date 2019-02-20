#pragma once

#include <Audio.h>
#include <map>
#include <vector>
#include <string>

class Audio
{
protected:

	std::unique_ptr<DirectX::AudioEngine> _audEngine;
	std::map<std::string, std::unique_ptr<DirectX::SoundEffect>> _effects;
	std::unique_ptr<DirectX::SoundEffectInstance> _effectInstance;
	//std::unique_ptr<DirectX::SoundEffectInstance> _effectInstance2;
	std::vector<std::string> _sequence;

	bool _retryAudio = false;
	bool _silentMode = false;
	bool _isPlaying = false;

	UINT _current = 0;

public:

	Audio();
	~Audio();

	void init();
	void addSoundEffect(const std::wstring& path, const std::string& name);
	void playWithEffect(const std::string& name, bool repeat = false);
	void update();

	void storeSequence(std::vector<std::string>& names);
	void playSequence();
};

