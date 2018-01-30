#pragma once

#include <string>
#include <list>
#include "TofuMath.h"
#include "ModelFormat.h"
#include <array>

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
		AnimationState(std::string name) : AnimNodeBase(name) {}
		//virtual ~AnimationState() {}

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight) override;

		virtual float GetDurationInSecond(Model *model) override;

		math::float3 LerpFromFrameIndex(Model * model, size_t lhs, size_t rhs) const;
		math::quat SlerpFromFrameIndex(Model * model, size_t lhs, size_t rhs) const;
	};

	class AnimationStateMachine : AnimNodeBase
	{

	protected:
		std::vector<AnimNodeBase*> states;
		std::unordered_map<std::string, uint16_t> stateIndexTable;

		AnimNodeBase *previous;
		AnimNodeBase *current;
		
		float transitionDuration;
		std::list<AnimationTransitionEntry> transitions;
		
		// Elapsed time since entering the current state
		float elapsedTime;

		//// Current Transition Index being evaluated
		//int32 EvaluatingTransitionIndex;

		//// The set of active transitions, if there are any
		//TArray<FAnimationActiveTransitionEntry> ActiveTransitionArray;

		//// The set of states in this state machine
		//TArray<FPoseLink> StatePoseLinks;

		//// Used during transitions to make sure we don't double tick a state if it appears multiple times
		//TArray<int32> StatesUpdated;

	private:
		// true if it is the first update.
		//bool bFirstUpdate;

		//TArray<FPoseContext*> StateCachedPoses;

		//FGraphTraversalCounter UpdateCounter;

		//TArray<FGraphTraversalCounter> StateCacheBoneCounters;

	public:
		AnimationStateMachine(std::string name);
		virtual ~AnimationStateMachine();

		void Play(std::string name);
		void CrossFade(std::string name, float normalizedTransitionDuration);

		AnimationState* AddState(std::string name);

		virtual void Enter(Model *model) override;
		virtual void Exit() override;

		virtual void Update(UpdateContext& context) override;
		virtual void Evaluate(EvaluateContext& context, float weight) override;

		virtual float GetDurationInSecond(Model *model) override;

		//void SetState(const FAnimationBaseContext& Context, int32 NewStateIndex);
		//void SetStateInternal(int32 NewStateIndex);

		//const int32 GetStateIndex(const FBakedAnimationState& StateInfo) const;


	//	// finds the highest priority valid transition, information pass via the OutPotentialTransition variable.
	//	// OutVisitedStateIndices will let you know what states were checked, but is also used to make sure we don't get stuck in an infinite loop or recheck states
	//	bool FindValidTransition(const FAnimationUpdateContext& Context,
	//		const FBakedAnimationState& StateInfo,
	//		/*OUT*/ FAnimationPotentialTransition& OutPotentialTransition,
	//		/*OUT*/ TArray<int32, TInlineAllocator<4>>& OutVisitedStateIndices);

	//	// Helper function that will update the states associated with a transition
	//	void UpdateTransitionStates(const FAnimationUpdateContext& Context, FAnimationActiveTransitionEntry& Transition);

	//	// helper functions for calling update and evaluate on state nodes
	//	void UpdateState(int32 StateIndex, const FAnimationUpdateContext& Context);
	//	const FPoseContext& EvaluateState(int32 StateIndex, const FPoseContext& Context);

	//	// transition type evaluation functions
	//	void EvaluateTransitionStandardBlend(FPoseContext& Output, FAnimationActiveTransitionEntry& Transition, bool bIntermediatePoseIsValid);
	//	void EvaluateTransitionStandardBlendInternal(FPoseContext& Output, FAnimationActiveTransitionEntry& Transition, const FPoseContext& PreviousStateResult, const FPoseContext& NextStateResult);
	//
	};
}