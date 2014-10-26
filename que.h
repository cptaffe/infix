//
// Copyright (c) 2013 Juan Palacios juan.palacios.puyana@gmail.com
// Subject to the BSD 2-Clause License
// - see < http://opensource.org/licenses/BSD-2-Clause>
//

#ifndef INFIX_QUE_H_
#define INFIX_QUE_H_

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class que {
 public:


  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
  }

  void pop(T& item) {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty() && !isDone())
    {
        // std::cout << "waiting" << std::endl;
      cond_.wait(mlock);
    }
    if (isDone() && queue_.empty()) {
        item = NULL;
        return;
    }
    item = queue_.front();
    queue_.pop();
  }

  void push(const T& item) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }

  void finish() {
    // std::cout << "finishing..." << std::endl;
     std::unique_lock<std::mutex> mlock(mutex_);
     done_ = true;
     mlock.unlock();
     cond_.notify_all();
    // std::cout << "finished" << std::endl;
  }

  bool isDone() {return done_;}
  bool isEmpty() {return queue_.empty();}
  que()=default;
  que(const que&) = delete;            // disable copying
  que& operator=(const que&) = delete; // disable assignment

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool done_ = false;
};


#endif // INFIX_QUE_H_
