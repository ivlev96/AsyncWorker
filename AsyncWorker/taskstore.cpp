#include "taskstore.h"

TaskStore::TaskStore()
{
}

TaskStore::~TaskStore()
{
}

void TaskStore::pushTask(std::function<void()>&& task)
{
	std::lock_guard<std::mutex> lock(m_accessMutex);
	m_tasks.emplace(std::move(task));
}

std::function<void()> TaskStore::popTask()
{
	std::lock_guard<std::mutex> lock(m_accessMutex);

	assert(m_tasks.size() > 0);
	const auto task = std::move(m_tasks.front());
	m_tasks.pop();

	return task;
}

std::size_t TaskStore::taskCount() const
{
	return m_tasks.size();
}

void TaskStore::clear()
{
	decltype(m_tasks) other;
	m_tasks.swap(other);
}
