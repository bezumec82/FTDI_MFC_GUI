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

    double immRXrate(size_t recv_bytes)
    {
		m_bytesOveral += recv_bytes;

		if (m_prevTime == 0)
		{
			m_prevTime = clock();
			return 0.0;
		}

		clock_t curr_time = clock();
		clock_t time_diff = curr_time - m_prevTime;

		double time_diff_msec = (double(time_diff))
			/ (double(CLOCKS_PER_SEC));

		m_prevTime = curr_time;

		return(double(recv_bytes) / time_diff_msec);
    }

	CString getImmRXrate(size_t recv_bytes)
	{
		std::wostringstream rx_rate_woss;
		rx_rate_woss << ::std::setprecision(6) << immRXrate(recv_bytes);
		return rx_rate_woss.str().c_str();
	}

	double medRXrate()
	{
		clock_t curr_time = clock();
		clock_t time_diff = curr_time - m_startTime;

		double time_diff_msec = (double(time_diff))
			/ (double(CLOCKS_PER_SEC));

		return (double(m_bytesOveral) / time_diff_msec);
	}

	CString getMedRXrate(size_t recv_bytes)
	{
		std::wostringstream rx_rate_woss;
		rx_rate_woss << ::std::setprecision(6) << medRXrate();
		return rx_rate_woss.str().c_str();
	}

    double mediumRXrate()
    {

    }

private:
	clock_t m_startTime{ 0 };
	clock_t m_prevTime{ 0 };
	
	uint64_t m_bytesOveral{ 0 };


	CString m_rate;
};

