#ifndef SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP
#define SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP

#include <future>
#include <type_traits> // std::invoke_result

class ThreadPool
{
  public:
    ThreadPool(int num_of_threads);

    template <typename F, typename... Args>
    auto add_task(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;
};

#endif // SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP
