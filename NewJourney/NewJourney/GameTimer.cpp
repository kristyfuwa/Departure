#include "GameTimer.h"
#include <windows.h>

GameTimer::GameTimer():m_bStop(false),m_iPausedTime(0),m_fDeltaTime(-1.0),m_fSecondsPerCount(0.0),m_iPrevTime(0),m_iCurrentTime(0)
{
	__int64		frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	m_fSecondsPerCount = 1.0 / (double)frequency;
}


GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{
	if (m_bStop)
	{
		return (float)((m_iStopTime - m_iPausedTime - m_iBaseTime)*m_fSecondsPerCount);
	}
	else
	{
		return (float)((m_iCurrentTime - m_iPausedTime - m_iBaseTime)*m_fSecondsPerCount);
	}
}

float GameTimer::DeltaTime() const
{
	return (float)m_fDeltaTime;
}

void GameTimer::Reset()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_iBaseTime = currentTime;
	m_iPrevTime = currentTime;;

	m_iStopTime = 0;
	m_bStop = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	if (m_bStop)
	{
		m_iPausedTime += (startTime - m_iStopTime);
		m_iPrevTime = startTime;
		m_iStopTime = 0;
		m_bStop = false;
	}
}

void GameTimer::Stop()
{
	if (!m_bStop)
	{
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		m_iStopTime = currentTime;
		m_bStop = true;
	}
}

void GameTimer::Tick()
{
	if (m_bStop)
	{
		m_fDeltaTime = 0.0;
		return;
	}
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	m_iCurrentTime = currentTime;
	
	// ��ǰ֡����һ֮֡���ʱ���
	m_fDeltaTime = (m_iCurrentTime - m_iPrevTime)*m_fSecondsPerCount;

	// Ϊ������һ֡��׼��
	m_iPrevTime = m_iCurrentTime;

	// ȷ����Ϊ��ֵ��DXSDK �е� CDXUTTimer �ᵽ����������������˽ڵ�ģʽ
	// ���л�����һ����������mDeltaTime ���Ϊ��ֵ��
	if (m_fDeltaTime < 0.0)
	{
		m_fDeltaTime = 0.0;
	}
}
