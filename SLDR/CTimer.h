#pragma once
#include "stdafx.h"

class CTimer

{

public:

	CTimer() {
		QueryPerformanceFrequency(&m_Frequency);
	}

	void Start() {
		QueryPerformanceCounter(&m_StartCount);
	}

	double End() {
		LARGE_INTEGER CurrentCount;
		QueryPerformanceCounter(&CurrentCount);
		return double(CurrentCount.LowPart - m_StartCount.LowPart) / (double)m_Frequency.LowPart * 1000;
	}

private:

	LARGE_INTEGER m_Frequency;

	LARGE_INTEGER m_StartCount;

};  