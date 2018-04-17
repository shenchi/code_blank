#include "AudioManager.h"
#include <Audio.h>

using namespace DirectX;

namespace tofu {
	/**
		AudioSource
	*/
	AudioSource::AudioSource(char* file)
	{
		size_t outSize;
		wchar_t wstr[256];
		mbstowcs_s(&outSize, wstr, 256, file, strlen(file));

		sound = std::make_unique<SoundEffect>(AudioManager::instance()->audEngine.get(), wstr);
		//soundEffect = std::make_unique<SoundEffect>(audEngine.get(), L"assets/sounds/steps.wav");
	}

	AudioSource::~AudioSource()
	{
	}

	void AudioSource::Play()
	{
		if (soundInstance == nullptr) {
			soundInstance = sound->CreateInstance();
		}

		// TODO: maintain background music list in AudioManager for volumn management

		soundInstance->SetVolume(AudioManager::instance()->volumn);
		soundInstance->Play();
	}

	void AudioSource::Stop()
	{
		if (soundInstance)
			soundInstance->Stop();
	}

	void AudioSource::PlayOneShot()
	{
		sound->Play(AudioManager::instance()->volumn, 0.f, 0.f);
	}

	/**
		Audio Manager
	*/
	SINGLETON_IMPL(AudioManager)

	AudioManager::AudioManager()
	{
		assert(nullptr == _instance);
		_instance = this;
	}

	AudioManager::~AudioManager()
	{
		if (audEngine)
			audEngine->Suspend();

		// reset loop background music
		// m_nightLoop.reset();
	}

	int32_t AudioManager::Init()
	{
		// This is only needed in Windows desktop apps
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags = eflags | AudioEngine_Debug;
#endif
		audEngine = std::make_unique<AudioEngine>(eflags);

		if (!audEngine->IsAudioDevicePresent())
		{
			// we are in 'silent mode'.
		}

		if (nullptr == audEngine)
			return kErrUnknown;

		return kOK;
	}

	int32_t AudioManager::Shutdown()
	{
		return kOK;
	}

	int32_t AudioManager::Update()
	{
		if (!audEngine->Update())
		{
			// No audio device is active
			if (audEngine->IsCriticalError())
			{
				// TODO:
			}
		}

		return kOK;
	}
}
