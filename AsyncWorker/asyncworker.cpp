#include "asyncworker.h"

bool AsyncWorker::s_threadsShouldStop = false;
std::atomic<std::size_t> AsyncWorker::s_threadsNotified = 0;

std::condition_variable AsyncWorker::s_wakeUpCondition;
std::mutex AsyncWorker::s_wakeUpMutex;

std::shared_ptr<TaskStore> AsyncWorker::s_taskStore = std::make_shared<TaskStore>();

AsyncWorker::AsyncWorker(std::size_t threadCount)
{
	for (std::size_t i = 0; i < threadCount; ++i)
	{
		m_threads.emplace_back(AsyncWorker::threadFunction);
	}
}

void AsyncWorker::threadFunction()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(s_wakeUpMutex);
		while (s_threadsNotified == 0) //to prevent spurious wakeups
		{
			s_wakeUpCondition.wait(lock);
		}
		--s_threadsNotified;
		lock.unlock();

		if (s_threadsShouldStop)
		{
			return;
		}
		s_taskStore->popTask()();
	}
}

void AsyncWorker::notifyOne()
{
	std::unique_lock<std::mutex> lock(wakeUpMutex());
	++s_threadsNotified;
	s_wakeUpCondition.notify_one();
}

const std::shared_ptr<TaskStore>& AsyncWorker::taskStore()
{
	return s_taskStore;
}

std::mutex& AsyncWorker::wakeUpMutex()
{
	return s_wakeUpMutex;
}

AsyncWorker& AsyncWorker::instance()
{
	static AsyncWorker worker;
	return (worker);
}

AsyncWorker::~AsyncWorker()
{
	std::unique_lock<std::mutex> lock(wakeUpMutex());
	s_taskStore->clear(); //remove all unstarted tasks

	s_threadsShouldStop = true;
	s_threadsNotified = m_threads.size();
	s_wakeUpCondition.notify_one();

	for (auto& thread : m_threads)
	{
		thread.join();
	}
	system("pause");
}