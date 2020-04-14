#pragma once
#include <ctime>
#include <cstdint>
#include <atlstr.h>

#include <iomanip> // std::setprecision
#include <sstream>

class TimeStat
{
public: /*--- Constructor ---*/
	TimeStat() = default;
public:
	void start()
	{
		m_prevTime = clock();
		m_startTime = m_prevTime;
	}

	void stop()
	{
		m_prevTime = { 0 };
		m_startTime = { 0 };
	}

    void reportStream(size_t bytes)
    {
		m_bytesOveral += bytes;
    }

	CString getMedByteRate()
	{
		clock_t curr_time = clock();
		clock_t time_diff = curr_time - m_startTime;
		double time_diff_msec = (double(time_diff))
			/ (double(CLOCKS_PER_SEC));
		return ::std::to_wstring(\
			(double(m_bytesOveral) / time_diff_msec) ).c_str();
	}

private:
	clock_t m_startTime{ 0 };
	clock_t m_prevTime{ 0 };
	uint64_t m_bytesOveral{ 0 };
	CString m_rate;
};

