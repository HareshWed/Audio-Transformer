#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadSafeQueue{
    public:
        void push(T value){
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
            cond_.notify_one();
        }

        std::optional<T> pop(){
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [&] {return !queue_.empty() || finished;});
            if (queue_.empty()) return std::nullopt;

            T value = std::move(queue_.front());
            queue_.pop();
            return value;
        }

        void setFinished(){
            std::lock_guard<std::mutex> lock(mutex_);
            finished = true;
            cond_.notify_all();
        }
    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
        bool finished = false;

};