#include "asyncworker.h"

AsyncWorker::AsyncWorker(std::size_t threadCount)
	: m_threadsShouldStop{ false }
	, m_threadsNotified{ 0 }
	, m_wakeUpCondition{}
	, m_wakeUpMutex{}
	, m_taskStore{ std::make_shared<TaskStore>() }
{
	for (std::size_t i = 0; i < threadCount; ++i)
	{
		m_threads.emplace_back(
			[this]()
			{
				while (true)
				{
					std::unique_lock<std::mutex> lock(m_wakeUpMutex);
					while (m_threadsNotified == 0) //to prevent spurious wakeups
					{
						m_wakeUpCondition.wait(lock);
					}
					--m_threadsNotified;

					if (m_threadsShouldStop)
					{
						return;
					}
					lock.unlock();

					m_taskStore->popTask()();
				}
			});
	}
}

AsyncWorker::~AsyncWorker()
{
	m_taskStore->clear(); //remove all unstarted tasks

	m_threadsShouldStop = true;
	m_threadsNotified = m_threads.size();
	m_wakeUpCondition.notify_all();

	for (auto& thread : m_threads)
	{
		thread.join();
	}
}

void AsyncWorker::notifyOne()
{
	std::unique_lock<std::mutex> lock(wakeUpMutex());
	++m_threadsNotified;
	m_wakeUpCondition.notify_one();
}

const std::shared_ptr<TaskStore>& AsyncWorker::taskStore()
{
	return m_taskStore;
}

std::mutex& AsyncWorker::wakeUpMutex()
{
	return m_wakeUpMutex;
}