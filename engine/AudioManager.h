#pragma once

#include "Common.h"
#include "Module.h"
#include <memory>

namespace DirectX
{
	class AudioEngine;
	class SoundEffect;
	class SoundEffectInstance;
	class SoundEffect;
}

namespace tofu {

	class AudioManager : public Module
	{
		SINGLETON_DECL(AudioManager)

	public:
		AudioManager();
		~AudioManager();

		int32_t Init() override;
		int32_t Shutdown() override;
		int32_t Update() override;

		void Play(const char * file);

	private:
		std::unique_ptr<DirectX::AudioEngine> audEngine;

		std::unique_ptr<DirectX::SoundEffect> soundEffect;
		std::unique_ptr<DirectX::SoundEffectInstance> effect;

		std::unique_ptr<DirectX::SoundEffect> soundEffect2;
	};
}