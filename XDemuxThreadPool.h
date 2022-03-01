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
	// ��ʼ���̳߳أ������߳�����
	void Init(int num);
	//�����̳߳�
	void Start();
	// ֹͣ�̳߳�
	void Stop();

	void AddTask(const std::shared_ptr<XTask>& task);

	std::shared_ptr<XTask> GetTask();

	bool is_exit() const { return is_exit_; }

	int task_run_count() { return task_run_count_; }

private:
	// �̳߳������߼�
	void Run();
	// �߳�����
	int thread_num_ = 0;
	// ������
	std::mutex mux_;
	// �߳�����
	std::vector<std::shared_ptr<std::thread>> threads_;
	// �������
	std::list<std::shared_ptr<XTask>> tasks_;

	std::condition_variable cv_;
	bool is_exit_ = false;
	std::atomic<int> task_run_count_ = { 0 };

};

