#pragma once

class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	float				TotalTime()const;
	float				DeltaTime()const;

	void				Reset();
	void				Start();
	void				Stop();
	void				Tick();
private:

	double				m_fSecondsPerCount;
	double				m_fDeltaTime;

	__int64				m_iBaseTime;
	__int64				m_iPausedTime;
	__int64				m_iStopTime;
	__int64				m_iPrevTime;
	__int64				m_iCurrentTime;

	bool				m_bStop;

};

