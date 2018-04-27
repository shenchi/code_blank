#pragma once

#include "Common.h"
#include "Module.h"
#include <memory>

namespace DirectX
{
	class AudioEngine;
	class SoundEffect;
	class SoundEffectInstance;
}

namespace tofu {
	class AudioSource {
	public:
		AudioSource(char* file, float volumn = 1.0f);
		~AudioSource();

		void Play();
		void Stop();

		void PlayOneShot();

	private:
		std::unique_ptr<DirectX::SoundEffect> sound;
		std::unique_ptr<DirectX::SoundEffectInstance> soundInstance = nullptr;

		float volumn;
	};

	class AudioManager : public Module
	{
		friend class AudioSource;
		SINGLETON_DECL(AudioManager)

	public:
		AudioManager();
		~AudioManager();

		int32_t Init() override;
		int32_t Shutdown() override;
		int32_t Update() override;

		float volumn = 1.0f;

	private:
		std::unique_ptr<DirectX::AudioEngine> audEngine;
	};
}