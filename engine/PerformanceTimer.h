#pragma once

#include "Common.h"

namespace tofu
{
	enum PerformanceTimerSlot
	{
		kPerformanceTimerSlotFrameTime,
		kPerformanceTimerSlotPhysicsTime,
		kPerformanceTimerSlotUserUpdateTime,
		kPerformanceTimerSlotRenderingSystemTime,
		kPerformanceTimerSlotCustem,
		kMaxPerformanceTimerSlot
	};
}

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1

#include "NativeContext.h"

namespace tofu
{
	struct PerformanceTimer
	{
		static int64_t counterFreq;
		static int64_t startTick;

		static int64_t timerSlots[kMaxPerformanceTimerSlot];
		static int64_t deltaTimerSlots[kMaxPerformanceTimerSlot];

		TF_INLINE static void Init() { 
			counterFreq = NativeContext::instance()->GetTimeCounterFrequency(); 
			startTick = NativeContext::instance()->GetTimeCounter();
			for (size_t i = 0; i < kMaxPerformanceTimerSlot; i++)
			{
				timerSlots[i] = startTick;
				deltaTimerSlots[i] = 0;
			}
		}

		TF_INLINE static int64_t GetTick()
		{
			return NativeContext::instance()->GetTimeCounter() - startTick;
		}

		// in ms
		TF_INLINE static float GetTime(int64_t ticks)
		{
			return (ticks / (float)counterFreq) * 1000.0f;
		}

		TF_INLINE static void Clear(uint32_t slot)
		{
			deltaTimerSlots[slot] = 0;
		}

		TF_INLINE static void Start(uint32_t slot)
		{
			deltaTimerSlots[slot] = 0;
			int64_t stamp = NativeContext::instance()->GetTimeCounter();
			timerSlots[slot] = stamp;
		}

		TF_INLINE static void RecordTick(uint32_t slot)
		{
			int64_t stamp = NativeContext::instance()->GetTimeCounter();
			deltaTimerSlots[slot] += (stamp - timerSlots[slot]);
			timerSlots[slot] = stamp;
		}

		TF_INLINE static void UpdateLastTick(uint32_t slot)
		{
			int64_t stamp = NativeContext::instance()->GetTimeCounter();
			timerSlots[slot] = stamp;
		}
	};
}

#define PERFORMANCE_TIMER_INIT() PerformanceTimer::Init();

#define PERFORMANCE_TIMER_CLEAR(SLOT) PerformanceTimer::Clear(SLOT);

#define PERFORMANCE_TIMER_START(SLOT) PerformanceTimer::Start(SLOT);

#define PERFORMANCE_TIMER_PAUSE(SLOT) PerformanceTimer::RecordTick(SLOT);

#define PERFORMANCE_TIMER_RESUME(SLOT) PerformanceTimer::UpdateLastTick(SLOT);

#define PERFORMANCE_TIMER_END(SLOT) PerformanceTimer::RecordTick(SLOT);

#else

#define PERFORMANCE_TIMER_INIT() 

#define PERFORMANCE_TIMER_CLEAR(SLOT) 

#define PERFORMANCE_TIMER_START(SLOT) 

#define PERFORMANCE_TIMER_PAUSE(SLOT)

#define PERFORMANCE_TIMER_RESUME(SLOT)

#define PERFORMANCE_TIMER_END(SLOT)

#endif