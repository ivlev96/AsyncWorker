#pragma once

class __declspec(dllexport) TaskStore
{
public:
	TaskStore();
	virtual ~TaskStore();

	void pushTask(std::function<void()>&& task);
	std::function<void()> popTask();

	std::size_t taskCount() const;
	void clear();

private:
	std::queue<std::function<void()>, std::list<std::function<void()>>> m_tasks;
	std::mutex m_accessMutex;
};