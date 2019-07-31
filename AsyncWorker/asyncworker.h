#pragma once

class __declspec(dllexport) AsyncWorker
{
public:
	static AsyncWorker& instance();

	template<typename FunctionType, typename CallbackType, typename... Args>
	typename std::enable_if_t<
		std::is_invocable_v<FunctionType, Args&&...> &&
		(std::is_void_v<std::invoke_result_t<FunctionType, Args&&...>> && std::is_invocable_v<CallbackType>
			|| std::is_invocable_v<CallbackType, std::invoke_result_t<FunctionType, Args&&...>>)
	>
		execute(FunctionType&& function, CallbackType&& callback, Args&&... args);

	virtual ~AsyncWorker();

protected:
	AsyncWorker(std::size_t threadCount = std::thread::hardware_concurrency());

private:
	std::size_t m_threadCount;
};

template<typename FunctionType, typename CallbackType, typename... Args>
typename std::enable_if_t<
	std::is_invocable_v<FunctionType, Args&&...> &&
	(std::is_void_v<std::invoke_result_t<FunctionType, Args&&...>> && std::is_invocable_v<CallbackType>
		|| std::is_invocable_v<CallbackType, std::invoke_result_t<FunctionType, Args&&...>>)
>
	AsyncWorker::execute(FunctionType&& function, CallbackType&& callback, Args&&... args)
{
	if constexpr (std::is_void_v<std::invoke_result_t<FunctionType, Args&&...>>)
	{
		std::forward<FunctionType>(function)(std::forward<Args>(args)...);
		std::forward<CallbackType>(callback)();
	}
	else
	{
		std::forward<CallbackType>(callback)(std::forward<FunctionType>(function)(std::forward<Args>(args)...));
	}
}