#include "pch.h"

#include "Audio.h"


Audio::Audio()
{
}


Audio::~Audio()
{
	if (_audEngine)	_audEngine->Suspend();
}



void Audio::init()
{
	const auto initialization_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (initialization_result != S_OK)
	{
		assert(false && "audio init failed");
		return;
	}

	//std::unique_ptr<AudioEngine> audEngine;
	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | DirectX::AudioEngine_Debug;
#endif
	_audEngine = std::make_unique<DirectX::AudioEngine>(eflags);

	if (!_audEngine->IsAudioDevicePresent())
	{
		_silentMode = true;
	}

	addSoundEffect(L"../Sounds/Cello/cello_a2.wav", "a");
	addSoundEffect(L"../Sounds/Cello/cello_b2.wav", "b");
	addSoundEffect(L"../Sounds/Cello/cello_c2.wav", "c");
	addSoundEffect(L"../Sounds/Cello/cello_d2.wav", "d");
	addSoundEffect(L"../Sounds/Cello/cello_e2.wav", "e");
	addSoundEffect(L"../Sounds/Cello/cello_f2.wav", "f");
	addSoundEffect(L"../Sounds/Cello/cello_g2.wav", "g");

	addSoundEffect(L"../Sounds/Cello/cello_ah.wav", "A");
	addSoundEffect(L"../Sounds/Cello/cello_bh.wav", "B");
	addSoundEffect(L"../Sounds/Cello/cello_ch.wav", "C");
	addSoundEffect(L"../Sounds/Cello/cello_dh.wav", "D");
	addSoundEffect(L"../Sounds/Cello/cello_eh.wav", "E");
	addSoundEffect(L"../Sounds/Cello/cello_fh.wav", "F");
	addSoundEffect(L"../Sounds/Cello/cello_gh.wav", "G");
}



void Audio::addSoundEffect(const std::wstring& path, const std::string& name)
{
	_effects.insert({ name, std::make_unique<DirectX::SoundEffect>(_audEngine.get(), path.c_str()) });
}



void Audio::playWithEffect(const std::string & name, bool repeat)
{
	_effectInstance = _effects.at(name)->CreateInstance(DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters);
	//_effectInstance2 = _effects.at("d")->CreateInstance(DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters);
	
	_effectInstance->Play(repeat);
	//_effectInstance2->Play(repeat);
	
	DirectX::AudioListener listener;
	listener.SetPosition(SVec3(0, 0, 0));
	DirectX::AudioEmitter emitter;
	emitter.SetPosition(SVec3(0, 0, 0));
	_effectInstance->Apply3D(listener, emitter, false);	
}



void Audio::update()
{
	/*
	if (_retryAudio)
	{
		_retryAudio = false;
		if (_audEngine->Reset())
		{
		}
	}
	else if (!_audEngine->Update())
	{
		if (_audEngine->IsCriticalError())
		{
			_retryAudio = true;
		}
	}*/
}



void Audio::storeSequence(std::vector<std::string>& names)
{
	_sequence = names;
}


void Audio::playSequence()
{
	if (!_effectInstance)
		_isPlaying = false;
	else
	{
		DirectX::SoundState state = _effectInstance->GetState();
		_isPlaying = state & DirectX::SoundState::PLAYING;
	}

	if(!_isPlaying)
	{
		playWithEffect(_sequence[_current]);
		_current = _current + 1 == _sequence.size() ? 0 : _current + 1;
		_isPlaying = true;
	}
}
