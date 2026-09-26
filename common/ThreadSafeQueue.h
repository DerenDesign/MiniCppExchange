#include <queue>
#include <mutex>
#include <condition_variable>


template <typename T>
class ThreadSafeQueue {

    private:
        std::queue<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cond;
    
    public:
        ThreadSafeQueue() = default;
        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

        void push(T val){
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(val));
            cond.notify_one();

        }

        void pop(T& val){
            std::unique_lock<std::mutex> lock(mutex_);
            cond.wait(lock, [this]{ return !queue_.empty(); });
            val = std::move(queue_.front());
            queue_.pop();

        }





};