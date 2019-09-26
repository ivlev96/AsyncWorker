#pragma once
#include "taskstore.h"

class __declspec(dllexport) AsyncWorker
{
public:
	AsyncWorker(std::size_t threadCount = std::thread::hardware_concurrency() - 1);
	virtual ~AsyncWorker();

	AsyncWorker(const AsyncWorker& other) = delete;
	AsyncWorker(AsyncWorker&& other) = delete;

	AsyncWorker& operator=(const AsyncWorker& other) = delete;
	AsyncWorker& operator=(AsyncWorker&& other) = delete;

	template<typename FunctionType, typename CallbackType, typename... Args>
	typename std::enable_if_t<std::is_invocable_v<FunctionType, Args...>>
		execute(FunctionType&& function, Args&&... args);

	template<typename FunctionType, typename CallbackType, typename... Args>
	typename std::enable_if_t<
		std::is_invocable_v<FunctionType, Args...> &&
		(std::is_void_v<std::invoke_result_t<FunctionType, Args...>> && std::is_invocable_v<CallbackType>
			|| std::is_invocable_v<CallbackType, std::invoke_result_t<FunctionType, Args...>>)
	>
		executeWithCallback(FunctionType&& function, CallbackType&& callback, Args&&... args);

private:
	void notifyOne();
	const std::shared_ptr<TaskStore>& taskStore();
	std::mutex& wakeUpMutex();

private:
	bool m_threadsShouldStop;
	std::atomic<std::size_t> m_threadsNotified;

	std::condition_variable m_wakeUpCondition;
	std::mutex m_wakeUpMutex;

	std::shared_ptr<TaskStore> m_taskStore;
	std::vector<std::thread> m_threads;
};

template<typename FunctionType, typename CallbackType, typename... Args>
typename std::enable_if_t<std::is_invocable_v<FunctionType, Args...>>
AsyncWorker::execute(FunctionType&& function, Args&&... args)
{
	taskStore()->pushTask(
		[_function = std::forward<FunctionType>(function),
		_args = std::make_tuple(args...)]()
	{
		std::apply(_function, _args);
	});

	notifyOne();
}

template<typename FunctionType, typename CallbackType, typename... Args>
typename std::enable_if_t<
	std::is_invocable_v<FunctionType, Args...> &&
	(std::is_void_v<std::invoke_result_t<FunctionType, Args...>> && std::is_invocable_v<CallbackType>
		|| std::is_invocable_v<CallbackType, std::invoke_result_t<FunctionType, Args...>>)
>
	AsyncWorker::executeWithCallback(FunctionType&& function, CallbackType&& callback, Args&&... args)
{
	if constexpr (std::is_void_v<std::invoke_result_t<FunctionType, Args...>>)
	{
		taskStore()->pushTask(
			[_function = std::forward<FunctionType>(function),
			 _callback = std::forward<CallbackType>(callback),
			 _args = std::make_tuple(args...)]()
			{
				std::apply(_function, _args);
				_callback();
			});
	}
	else
	{
		taskStore()->pushTask(
			[_function = std::forward<FunctionType>(function),
			_callback = std::forward<CallbackType>(callback),
			_args = std::make_tuple(args...)]()
		{
			std::invoke(_callback, std::apply(_function, _args));
		});
	}

	notifyOne();
}