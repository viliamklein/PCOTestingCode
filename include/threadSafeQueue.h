#ifndef THREADSAFEQUEUE_HEADER
#define THREADSAFEQUEUE_HEADER

#include <queue>
#include <mutex>
#include <stdexcept>

#define IMGQUEMAXLEN 10
#define MSGQUEMAXLEN 10

// template <typename T, int MaxLen, typename Container=std::deque<T>>
// class FixedQueue : public std::queue<T, Container> {
// public:
//     void push(const T& value) {
      
//         std::lock_guard<std::mutex> lock(mutex_);

//         if (this->size() == MaxLen) {
//            this->c.pop_front();
//         }
//         // std::queue<T, Container>::push(value);
//         queue_.push(value);
//     }
// };


template<typename T, int MaxLen>
class ThreadsafeQueue {
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  const int queLength = MaxLen;
 
  // Moved out of public interface to prevent races between this
  // and pop().
  bool empty() const {
    return queue_.empty();
  }
 
 public:
  ThreadsafeQueue() = default;
  ThreadsafeQueue(const ThreadsafeQueue<T, MaxLen> &) = delete ;
  ThreadsafeQueue& operator=(const ThreadsafeQueue<T, MaxLen> &) = delete ;
 
  ThreadsafeQueue(ThreadsafeQueue<T, MaxLen>&& other) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_ = std::move(other.queue_);
  }
 
  virtual ~ThreadsafeQueue() { }
 
  unsigned long size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }
 
  std::optional<T> pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return {};
    }
    T tmp = queue_.front();
    queue_.pop();
    return tmp;
  }
 
  void push(const T &item) {

    if (this->size() == queLength) {
      // this->queue_.pop_front();
      throw std::overflow_error("que overflow");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);

  }
};

#endif