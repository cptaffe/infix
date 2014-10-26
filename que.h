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
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
	bool done_ = false;
public:
	T pop();
	void pop(T& item);
	void push(const T& item);
	void finish();
	bool isDone();
	bool isEmpty();
};

template <typename T>
T que<T>::pop() {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		cond_.wait(mlock);
	}
	auto val = queue_.front();
	queue_.pop();
	return val;
}

template <typename T>
void que<T>::pop(T& item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty() && !isDone()) {
		cond_.wait(mlock);
	}
	if (isDone() && queue_.empty()) {
		item = NULL;
		return;
	}
	item = queue_.front();
	queue_.pop();
}

template <typename T>
void que<T>::push(const T& item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(item);
	mlock.unlock();
	cond_.notify_one();
}

template <typename T>
void que<T>::finish() {
	std::unique_lock<std::mutex> mlock(mutex_);
	done_ = true;
	mlock.unlock();
	cond_.notify_all();
}

template <typename T>
bool que<T>::isDone() {return done_;}

template <typename T>
bool que<T>::isEmpty() {return queue_.empty();}


#endif // INFIX_QUE_H_
