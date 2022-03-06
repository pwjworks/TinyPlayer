#pragma once
#include <QThread>
#include <mutex>
#include <list>
class XDecode;

class AVPacketRAII;

class XDecodeThread : public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	virtual void Push(std::shared_ptr<AVPacketRAII> pkt);

	// 清理队列
	virtual void Clear();
	// 清理资源，停止线程
	virtual void Close();

	// 取出一帧数据，并出栈，如果没有返回NULL
	virtual std::shared_ptr<AVPacketRAII> Pop();
	//最大队列
	int maxList = 200;
	bool isExit = false;


protected:
	XDecode* decode = 0;
	std::list <std::shared_ptr<AVPacketRAII>> packs;
	std::mutex mux;

};

