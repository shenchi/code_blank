#pragma once

#include <string>
#include <list>
#include "TofuMath.h"
#include "ModelFormat.h"

class Transform;

namespace tofu
{
	enum AnimationEvaluationType
	{
		kAET_Blend,
		kAET_Additive,
		kAET_Override
	};

	class Model;

	struct AnimationTransitionEntry
	{
		std::string name;
		float normalizedDuration;
	};

	struct UpdateContext
	{
		Model *model;
		std::list<AnimationTransitionEntry> *transitions;
	};

	struct EvaluateContext
	{
		Model *model;
		Transform *results;
		std::vector<uint16_t> *selectedJoints = nullptr;
	
		EvaluateContext(Model *model);
		~EvaluateContext();
	};

	class AnimationFrameCache
	{
		friend class AnimationState;
		friend class AnimationStateCache;

	private:
		size_t indices[3][4];

	public:
		AnimationFrameCache() { Reset(); }

		void Reset();
		void AddFrameIndex(model::ChannelType type, size_t index);
	};

	class AnimationStateCache
	{
		friend class AnimationState;

	private:
		float ticks;

		// progress of playback, [0..1]
		float progress;

		// current position in key frames (for linear scan)
		size_t cursor;

		// cache to keep t-1 to t+2 key frame index from previous search
		std::vector<AnimationFrameCache> frameCaches;

	public:
		AnimationStateCache() { Reset(); }
		void Reset();

		void Update(UpdateContext* context, tofu::model::ModelAnimation* animation);
	};

	class AnimNodeBase
	{

	public:
		std::string name;

	public:
		AnimNodeBase() = default;
		AnimNodeBase(std::string name);
		AnimNodeBase& operator=(AnimNodeBase other) { swap(*this, other); return *this; }
		AnimNodeBase(AnimNodeBase& other) = delete;
		AnimNodeBase(AnimNodeBase&& other) noexcept : AnimNodeBase() { swap(*this, other); }
		virtual ~AnimNodeBase() {}

		friend void swap(AnimNodeBase& lhs, AnimNodeBase& rhs) noexcept;

		virtual void Enter(Model *model) {}
		virtual void Exit() {}

		virtual void Update(UpdateContext& context) {}
		virtual void Evaluate(EvaluateContext& context, float weight, AnimationEvaluationType type) {}

		virtual float GetPlaybackProgress() const{ return 0.0f; }

		virtual float GetDurationInSecond(Model *model) { return 0.f; }
	};

	class AnimationState : AnimNodeBase
	{
		friend class AnimationStateMachine;

	public:
		bool isLoop = true;
		float playbackSpeed = 1.0f;

		AnimationStateCache* cache = nullptr;

		std::string	animationName = "";
	public:
		AnimationState() = default;
		AnimationState(std::string name, bool loop = true) : AnimNodeBase(name) , isLoop(loop) {}
		AnimationState& operator=(AnimationState other) { swap(*this, other); return *this; }
		AnimationState(AnimationState& other) = delete;
		AnimationState(AnimationState&& other) noexcept : AnimationState() { swap(*this, other); }
		virtual ~AnimationState();

		friend void swap(AnimationState& lhs, AnimationState& rhs) noexcept;

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight, AnimationEvaluationType type) override;

		virtual float GetPlaybackProgress() const override;

		virtual float GetDurationInSecond(Model *model) override;

	private:
		template<typename T, typename GetFuncType, typename LinearFuncType, typename CubicFuncType>
		void SetTransform(size_t * indices, Model * model, Transform & trans, Transform & tPose, void(Transform::* set)(const T &), GetFuncType get, LinearFuncType linear, CubicFuncType cubic);

		math::float3 LerpFrame(Model * model, size_t lhs, size_t rhs) const;
		math::quat SlerpFrame(Model * model, size_t lhs, size_t rhs) const;
		math::float3 CatmullRomFrame(Model * model, size_t i1, size_t i2, size_t i3, size_t i4) const;
		math::quat SquadFrame(Model * model, size_t i1, size_t i2, size_t i3, size_t i4) const;

		TF_INLINE void InternalEvaluate(uint16_t i, EvaluateContext & context, float weight, AnimationEvaluationType type);
	};

	class AnimationStateMachine : AnimNodeBase
	{

	private:
		std::vector<AnimNodeBase*> states;
		std::unordered_map<std::string, uint16_t> stateIndexTable;

		AnimNodeBase *previous = nullptr;
		AnimNodeBase *current = nullptr;

		// Elapsed time since entering the current state
		float elapsedTime = 0.f;
		float transitionDuration = 0.f;

		std::list<AnimationTransitionEntry> transitions;

	public:
		AnimationStateMachine() = default;
		AnimationStateMachine(std::string name);
		AnimationStateMachine& operator=(AnimationStateMachine other) { swap(*this, other); return *this; }
		AnimationStateMachine(AnimationStateMachine& other) = delete;
		AnimationStateMachine(AnimationStateMachine&& other) noexcept : AnimationStateMachine() { swap(*this, other); }
		virtual ~AnimationStateMachine();

		friend void swap(AnimationStateMachine& lhs, AnimationStateMachine& rhs) noexcept;

		void Play(std::string name);
		void CrossFade(std::string name, float normalizedTransitionDuration);

		AnimationState* AddState(std::string name, bool isLoop = true);

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight, AnimationEvaluationType type) override;

		virtual float GetPlaybackProgress() const override;

		virtual float GetDurationInSecond(Model *model) override;

	private:
		TF_INLINE void InternalEvaluate(uint16_t i, EvaluateContext & context, float weight, AnimationEvaluationType type);
	};

	class AnimationLayer {
		friend class AnimationComponentData;

	public:
		AnimationLayer(std::string name, float weight = 1.0f, AnimationEvaluationType type = kAET_Blend);

		virtual void Update(Model *model);
		virtual void Evaluate(EvaluateContext& context);

		AnimationStateMachine *GetStateMachine() { return &stateMachine; }

		// FIXME:
		std::vector<uint16_t> *selectedJoints = nullptr;

	private:
		std::string name;
		float weight;
		AnimationEvaluationType type;

		AnimationStateMachine stateMachine;
	};
}