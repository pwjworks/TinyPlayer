#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

class XTask {
public:
	virtual int Run() = 0;
	std::function<bool()> is_exit = nullptr;
	auto GetReturn() {
		return p_.get_future().get();
	}
	void SetValue(int v) {
		p_.set_value(v);
	}

private:
	std::promise<int> p_;
};


class XDemuxThreadPool
{
public:
	XDemuxThreadPool();
	~XDemuxThreadPool();
	// 初始化线程池，设置线程数量
	void Init(int num);
	//启动线程池
	void Start();
	// 停止线程池
	void Stop();

	void AddTask(const std::shared_ptr<XTask>& task);

	std::shared_ptr<XTask> GetTask();

	bool is_exit() const { return is_exit_; }

	int task_run_count() { return task_run_count_; }

private:
	// 线程池运行逻辑
	void Run();
	// 线程数量
	int thread_num_ = 0;
	// 互斥锁
	std::mutex mux_;
	// 线程数组
	std::vector<std::shared_ptr<std::thread>> threads_;
	// 任务队列
	std::list<std::shared_ptr<XTask>> tasks_;

	std::condition_variable cv_;
	bool is_exit_ = false;
	std::atomic<int> task_run_count_ = { 0 };

};

