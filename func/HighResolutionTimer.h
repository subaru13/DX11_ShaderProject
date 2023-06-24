#pragma once
#include <chrono>

class HighResolutionTimer
{
public:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration = Clock::duration;
	using Period = Clock::period;
	using Rep = Clock::rep;
private:
	static constexpr double MSecondsPerCount = 1.0 / static_cast<double>(Period::den);
private:
	const TimePoint mRecordStartPoint;
	TimePoint mPreviousRecordPoint;
	TimePoint mSleepPoint;
	double mTimeOfSleeping;
	double mElapsedTime;
	bool mIsSleeping;
public:
	HighResolutionTimer();

	virtual ~HighResolutionTimer() = default;

	double GetElapsedTime() const;

	bool IsSleeping() const;

	void BeginSleeping();

	void EndSleeping();

	void Tick();
};
