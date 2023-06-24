#include "HighResolutionTimer.h"

HighResolutionTimer::HighResolutionTimer()
	:mRecordStartPoint(Clock::now()),
	mPreviousRecordPoint(),
	mSleepPoint(),
	mTimeOfSleeping(),
	mElapsedTime(),
	mIsSleeping(false)
{
	mPreviousRecordPoint = mRecordStartPoint;
}

double HighResolutionTimer::GetElapsedTime() const
{
	return mElapsedTime;
}

bool HighResolutionTimer::IsSleeping() const
{
	return mIsSleeping;
}

void HighResolutionTimer::BeginSleeping()
{
	mIsSleeping = true;
	mSleepPoint = Clock::now();
}

void HighResolutionTimer::EndSleeping()
{
	const TimePoint thisTimePoint = Clock::now();
	const Duration sleepDuration = thisTimePoint - mSleepPoint;
	mTimeOfSleeping = sleepDuration.count() * MSecondsPerCount;
	mIsSleeping = false;
}

void HighResolutionTimer::Tick()
{
	if (mIsSleeping)
	{
		mElapsedTime = 0;
		return;
	}
	const TimePoint thisTimePoint = Clock::now();
	const Duration elapsedDuration = thisTimePoint - mPreviousRecordPoint;
	mElapsedTime = elapsedDuration.count() * MSecondsPerCount;
	mElapsedTime -= mTimeOfSleeping;
	mTimeOfSleeping = 0;
	mPreviousRecordPoint = thisTimePoint;
}
