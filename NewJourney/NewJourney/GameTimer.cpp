#include "GameTimer.h"
#include <windows.h>

GameTimer::GameTimer():m_bStop(false),m_iPausedTime(0),m_fDeltaTime(-1.0),m_fSecondsPerCount(0.0),m_iPrevTime(0),m_iCurrentTime(0)
{
	__int64		frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	m_fSecondsPerCount = 1.0f / (double)frequency;
}


GameTimer::~GameTimer()
{
}

void GameTimer::Tick()
{
	if (m_bStop)
	{
		m_fDeltaTime = 0.0;
		return;
	}
	__int64 currentTime;
	QueryPerformanceCounter(LARGE_INTEGER*)&currentTime);
}
