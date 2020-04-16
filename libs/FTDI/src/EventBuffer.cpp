#include "ftdi.h"

using namespace FTDI;

void EventBuffer::doBuffer()
{
	auto work = [&]()
	{
		Events events;
		m_isWorking.store(true);
		while (!m_stop.load())
		{
			{
				::std::unique_lock<::std::mutex> lock(m_eventsLock);
				events.swap(m_events);
			}
			while (!events.empty())
			{
				{
					::std::unique_lock<::std::mutex> lock(m_callBacksLock);
					for (const auto& func : m_callBacks)
						func(events.front().first, events.front().second);
				}
				events.pop();
			}
			::std::this_thread::sleep_for(\
				::std::chrono::milliseconds(BUFFER_DELAY_MS));
		} //end while
		m_isWorking.store(false);
		::std::cout << "Event passing is stopped" << ::std::endl;
	};

	m_future = ::std::async(::std::launch::async, work);
}

void EventBuffer::start()
{
	if (isWorking())
	{
		::std::cerr << "Repeated start" << ::std::endl;
		return;
	}
	m_stop.store(false);
	doBuffer();
	::std::cout << "Event buffer is started" << ::std::endl;
};

void EventBuffer::stop()
{
	m_stop.store(true);
	{
		::std::unique_lock<::std::mutex> lock(m_eventsLock);
		while (!m_events.empty()) //clear queue
			m_events.pop();
	}
	::std::cout << "Event buffer is stopped" << ::std::endl;
}

void EventBuffer::registerCallBack(::std::function <CallBack> call_back)
{
	::std::unique_lock<::std::mutex> lock(m_callBacksLock);
	m_callBacks.emplace_back(call_back);
}

void EventBuffer::receiveEvent(const ::FTDI::EventCode& event,
	const ::FTDI::Data& data)
{
	::std::unique_lock<::std::mutex> lock(m_eventsLock);
	if (!m_stop.load())
		m_events.emplace(Event{ event, data });
}
