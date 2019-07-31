#include "asyncworker.h"

AsyncWorker::AsyncWorker(std::size_t threadCount)
	: m_threadCount(threadCount)
{
}

AsyncWorker& AsyncWorker::instance()
{
	static AsyncWorker worker;
	return (worker);
}

AsyncWorker::~AsyncWorker()
{
}

