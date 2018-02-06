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
		kAET_None,
		kAET_Additive
	};

	class Model;
	class AnimNodeBase;

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
		Transform *transforms;

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
		std::string	name;

	public:
		AnimNodeBase(std::string name);
		virtual ~AnimNodeBase() {}

		virtual void Enter(Model *model) {}
		virtual void Exit() {}

		virtual void Update(UpdateContext& context) {}
		virtual void Evaluate(EvaluateContext& context, float weight) {}

		virtual float GetDurationInSecond(Model *model) { return 0.f; }
	};

	class AnimationState : AnimNodeBase
	{
		friend class AnimationStateMachine;

	public:
		std::string	animationName;

		bool isLoop = true;
		float playbackSpeed = 1.0f;

		AnimationStateCache* cache;
	public:
		AnimationState(std::string name = "state") : AnimNodeBase(name) {}
		virtual ~AnimationState();

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight) override;

		virtual float GetDurationInSecond(Model *model) override;

		math::float3 CatmullRomIndex(Model * model, size_t i1, size_t i2, size_t i3, size_t i4) const;
		math::quat SquadIndex(Model * model, size_t i1, size_t i2, size_t i3, size_t i4) const;
		math::float3 LerpFromFrameIndex(Model * model, size_t lhs, size_t rhs) const;
		math::quat SlerpFromFrameIndex(Model * model, size_t lhs, size_t rhs) const;
	};

	class AnimationStateMachine : AnimNodeBase
	{

	private:
		std::vector<AnimNodeBase*> states;
		std::unordered_map<std::string, uint16_t> stateIndexTable;

		AnimNodeBase *previous;
		AnimNodeBase *current;

		// Elapsed time since entering the current state
		float elapsedTime;

		float transitionDuration;
		std::list<AnimationTransitionEntry> transitions;

	public:
		AnimationStateMachine(std::string name = "machine");
		// TODO::
		//AnimationStateMachine(const AnimationStateMachine& other);
		AnimationStateMachine(AnimationStateMachine && other) noexcept;
		virtual ~AnimationStateMachine();

		void Play(std::string name);
		void CrossFade(std::string name, float normalizedTransitionDuration);

		AnimationState* AddState(std::string name);

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight) override;

		virtual float GetDurationInSecond(Model *model) override;
	};

	class AnimationLayer {
		friend class AnimationComponentData;

	public:
		AnimationLayer(std::string name, float weight = 1.0f);

		virtual void Update(Model *model);
		virtual void Evaluate(EvaluateContext& context);

	private:
		std::string name;
		float weight;
		AnimationStateMachine stateMachine;
	};
}