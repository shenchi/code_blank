#include "AudioManager.h"
#include <cassert>

using namespace DirectX;

namespace tofu {
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

		soundEffect = std::make_unique<SoundEffect>(audEngine.get(), L"assets/sounds/steps.wav");
		soundEffect2 = std::make_unique<SoundEffect>(audEngine.get(), L"assets/sounds/game-sound-correct.wav");
		effect = soundEffect->CreateInstance();
		//effect->SetVolume(0.1f);
		effect->Play(true);

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

	void AudioManager::Play(const char * file)
	{
		size_t outSize;

		wchar_t wstr[256];
		mbstowcs_s(&outSize, wstr, 256, file, strlen(file));

		//soundEffect = std::make_unique<SoundEffect>(audEngine.get(), wstr);
		soundEffect2->Play(0.01f, 0.f, 0.f);
	}
}
