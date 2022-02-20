#pragma once
#include <QThread>
#include "IVideoCall.h"
#include <mutex>
class XDemux;
class XVideoThread;
class XAudioThread;
class XDemuxThread :public QThread
{
public:
	//创建对象并打开
	virtual bool Open(const char* url, IVideoCall* call);

	//启动所有线程
	virtual void Start();

	virtual void Close();
	void run();
	XDemuxThread();
	virtual ~XDemuxThread();
	bool isExit = false;

	long long pts = 0;
	long long totalMs = 0;

	void SetPause(bool isPause);
	bool isPause = false;
protected:
	std::mutex mux;
	XDemux* demux = 0;
	XVideoThread* vt = 0;
	XAudioThread* at = 0;
};



