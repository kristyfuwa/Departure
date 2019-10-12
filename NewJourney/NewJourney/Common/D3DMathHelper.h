#pragma once
#include <windows.h>

class D3DMathHelper
{
public:
	D3DMathHelper();
	~D3DMathHelper();

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	template<typename T>
	static T Min(const T& x, const T& y)
	{
		return x <= y ? x : y;
	}

	template<typename T>
	static T Max(const T& x, const T& y)
	{
		return x >= y ? x : y;
	}

	static const float Infinity;
	static const float PI;
};

