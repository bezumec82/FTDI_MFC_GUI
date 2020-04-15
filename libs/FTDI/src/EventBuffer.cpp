#include "ftdi.h"

using namespace FTDI;

EventBuffer::EventBuffer()
{
	auto work = [&]()
	{
		while (!m_stop.load())
		{
			Events events;
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
	};

	m_future = ::std::async(::std::launch::async, work);
}

EventBuffer::~EventBuffer()
{
	m_stop.store(true);
	m_future.get();
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
	m_events.emplace(Event{ event, data });
}
