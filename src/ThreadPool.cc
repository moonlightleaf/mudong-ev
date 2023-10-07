#include "ThreadPool.hpp"
#include "Logger.hpp"

using namespace mudong::ev;

ThreadPool::ThreadPool(size_t threadNum, size_t maxQueueSize, const ThreadInitCallback& callback)
        : maxQueueSize_(maxQueueSize),
          running_(true),
          threadInitCallback_(callback)
{
    assert(maxQueueSize_ > 0);
    for (size_t i = 0; i < threadNum; ++i) { // 从0开始标号，作为线程池中线程的标识
        threads_.emplace_back(new std::thread([i, this](){runInThread(i);}));
    }
    TRACE("ThreadPool construct threadNum {}, maxQueueSize {}", threadNum, maxQueueSize);
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
    TRACE("ThreadPool destruct");
}

void ThreadPool::runTask(const Task& task) {
    assert(running_);
    if (threads_.empty()) {
        task();
    }
    else {
        std::unique_lock<std::mutex> lock(mutex_);
        while (taskQueue_.size() >= maxQueueSize_) {
            notFull_.wait(lock);
        }
        taskQueue_.push(task);
        notEmpty_.notify_one();
    }
}

void ThreadPool::runTask(Task&& task) {
    assert(running_);
    if (threads_.empty()) {
        task();
    }
    else {
        std::unique_lock<std::mutex> lock(mutex_);
        while (taskQueue_.size() >= maxQueueSize_) {
            notFull_.wait(lock);
        }
        taskQueue_.push(std::move(task));
        notEmpty_.notify_one();
    }
}

void ThreadPool::stop() {
    assert(running_);
    running_ = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        notEmpty_.notify_all();
    }
    for (auto& thread : threads_) {
        thread->join();
    }
}

size_t ThreadPool::threadNum() const {
    return threads_.size();
}

void ThreadPool::runInThread(size_t index) {
    if (threadInitCallback_) {
        threadInitCallback_(index);
    }
    while (running_) {
        if (Task task = take()) {
            task();
        }
    }
}

Task ThreadPool::take() {
    std::unique_lock<std::mutex> lock(mutex_);
    // 确保能在停止指令发出时，正确停止，返回一个空Task
    while (taskQueue_.empty() && running_) {
        notEmpty_.wait(lock);
    }
    Task task;
    if (!taskQueue_.empty()) {
        task = taskQueue_.front();
        taskQueue_.pop();
        notFull_.notify_one();
    }
    return task;
}