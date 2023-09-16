#pragma once

#include <thread>
#include <condition_variable>
#include <queue>

#include "noncopyable.hpp"
#include "Callbacks.hpp"

namespace mudong {

namespace ev {

class ThreadPool: noncopyable {

public:
    explicit ThreadPool(size_t threadNum, size_t maxQueueSize = 65536, const ThreadInitCallback& callback = nullptr);
    ~ThreadPool();

    void runTask(const Task&);
    void runTask(Task&&);
    void stop();
    size_t threadNum() const;

private:
    void runInThread(size_t index);
    Task take();

    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadList = std::vector<ThreadPtr>;

    ThreadList threads_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::queue<Task> taskQueue_;
    const size_t maxQueueSize_;
    std::atomic_bool running_;
    ThreadInitCallback threadInitCallback_;
};

} // namespace ev

} // namespace mudong