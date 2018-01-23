#pragma once

#include <string>
#include <vector>
#include "TofuMath.h"
#include "ModelFormat.h"

namespace tofu
{
	class Model;

	struct PoseContext 
	{
		math::float4x4* Transformation;
		uint16_t numBones;
	};

	struct AnimNodeBase
	{
		virtual ~AnimNodeBase() {}

		virtual void Update(Model* model);
		virtual void Evaluate(Model* model, PoseContext& Output);
	};

	class AnimationFrameCache
	{
		friend class AnimationComponentData;

	public:
		AnimationFrameCache() {
			Reset();
		}

	private:
		size_t indices[3][4];

	private:
		void Reset();
		void AddFrameIndex(model::ChannelType type, size_t index);
	};

	struct AnimationStateCache
	{
		float					ticks;

		// current position in key frames (for linear scan)
		uint32_t				cursor;
		
		// cache to keep t-1 to t+2 key frame index from previous search
		AnimationFrameCache		caches;
	};

	struct AnimationState : AnimNodeBase
	{
	public:
		AnimationState();
		~AnimationState();

	private:
		std::string				stateName;
		std::string				animationName;

		AnimationStateCache*	cache;

		float					playbackSpeed;
		bool					isLoop;

	public:
		virtual void Update(Model* model) override;
		virtual void Evaluate(Model* model, PoseContext& Output) override;
	};

	struct AnimationStateMachine : AnimNodeBase 
	{
		friend class AnimationComponent;
		
	protected:
		std::vector<AnimationState> animationStates;

		uint16_t currentState;

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
		virtual void Update(Model* model) override;
		virtual void Evaluate(Model* model, PoseContext& Output) override;

	

	protected:

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