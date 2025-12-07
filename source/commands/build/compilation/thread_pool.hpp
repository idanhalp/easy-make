#ifndef SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP
#define SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP

#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <type_traits> // std::invoke_result
#include <vector>

class ThreadPool
{
  public:
    ThreadPool(int num_of_threads);
    ~ThreadPool();

    template <typename F, typename... Args>
    auto add_task(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

  private:
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> task_queue;
    std::condition_variable condition;
    std::mutex queue_mutex;
};

ThreadPool::ThreadPool(const int num_of_threads)
{
    assert(num_of_threads > 0);

    for (auto i = 0; i < num_of_threads; ++i)
    {
        workers.emplace_back(
            [this](std::stop_token stop_token)
            {
                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(
                            lock, [this, stop_token] { return stop_token.stop_requested() || !task_queue.empty(); });

                        const auto all_tasks_complete = stop_token.stop_requested() && task_queue.empty();

                        if (all_tasks_complete)
                        {
                            return;
                        }

                        if (!task_queue.empty())
                        {
                            task = std::move(task_queue.front());
                            task_queue.pop();
                        }
                    }

                    if (task)
                    {
                        task();
                    }
                }
            });
    }
}

ThreadPool::~ThreadPool()
{
    for (auto& worker : workers)
    {
        worker.request_stop();
    }

    condition.notify_all();
}

template <typename F, typename... Args>
auto ThreadPool::add_task(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = std::invoke_result_t<F, Args...>;

    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    auto result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        task_queue.emplace([task]() { (*task)(); });
    }

    condition.notify_one();

    return result;
}

#endif // SOURCE_COMMANDS_BUILD_COMPILATION_THREAD_POOL_HPP
