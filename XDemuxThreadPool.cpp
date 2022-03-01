#include "XDemuxThreadPool.h"
#include <iostream>
using namespace std;

void XDemuxThreadPool::Init(int num) {
	unique_lock<mutex> lock(mux_);
	this->thread_num_ = num;
	cout << "Thread pool Init! " << num << endl;
}

void XDemuxThreadPool::Start() {
	unique_lock<mutex> lock(mux_);
	if (thread_num_ <= 0) {
		cerr << "Please Init XthreadPool" << endl;
		return;
	}
	if (!threads_.empty()) {
		cerr << "Thread pool has start!" << endl;
		return;
	}
	for (int i = 0; i < thread_num_; i++) {
		//auto th = new thread(&XThreadPool::Run, this);
		auto th = make_shared<thread>(&XDemuxThreadPool::Run, this);
		threads_.emplace_back(th);
	}
}

void XDemuxThreadPool::Stop() {
	is_exit_ = true;
	cv_.notify_all();
	for (auto& th : threads_) {
		th->join();
	}
	unique_lock<mutex> lock(mux_);
	threads_.clear();
}

void XDemuxThreadPool::Run() {
	while (!is_exit()) {
		auto task = GetTask();
		if (!task) continue;
		task_run_count_++;
		try {
			auto re = task->Run();
			task->SetValue(re);
		}
		catch (...) {
		}
		--task_run_count_;
	}
}

void  XDemuxThreadPool::AddTask(const std::shared_ptr<XTask>& task) {
	unique_lock<mutex> lock(mux_);
	task->is_exit = [this] { return is_exit(); };
	tasks_.emplace_back(task);
	lock.unlock();
	cv_.notify_one();
}

std::shared_ptr<XTask> XDemuxThreadPool::GetTask() {
	unique_lock<mutex> lock(mux_);
	if (tasks_.empty()) {
		cv_.wait(lock);
	}
	if (is_exit())
		return nullptr;
	if (tasks_.empty())
		return nullptr;
	auto task = std::move(tasks_.front());
	tasks_.pop_front();
	return task;
}