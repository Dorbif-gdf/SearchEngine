
#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t threadCount)
    {
        if (threadCount == 0)
        {
            threadCount = 1;
        }

        for (std::size_t i = 0; i < threadCount; ++i)
        {
            workers_.emplace_back([this]()
                {
                    workerLoop();
                });
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }

        condition_.notify_all();

        for (std::thread& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    template <typename Function, typename... Args>
    auto submit(Function&& function, Args&&... args)
        -> std::future<std::invoke_result_t<Function, Args...>>
    {
        using ReturnType = std::invoke_result_t<Function, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(
                std::forward<Function>(function),
                std::forward<Args>(args)...
            )
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopping_)
            {
                throw std::runtime_error(
                    "Cannot submit task: thread pool is stopping");
            }

            tasks_.emplace([task]()
                {
                    (*task)();
                });
        }

        condition_.notify_one();

        return result;
    }

private:
    void workerLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);

                condition_.wait(lock, [this]()
                    {
                        return stopping_ || !tasks_.empty();
                    });

                if (stopping_ && tasks_.empty())
                {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mutex_;
    std::condition_variable condition_;

    bool stopping_ = false;
};
