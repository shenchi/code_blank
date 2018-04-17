#include "PerformanceTimer.h"

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1

namespace tofu
{
	int64_t PerformanceTimer::counterFreq = 0;
	int64_t PerformanceTimer::startTick = 0;

	int64_t PerformanceTimer::timerSlots[kMaxPerformanceTimerSlot];
	int64_t PerformanceTimer::deltaTimerSlots[kMaxPerformanceTimerSlot];
}

#endif