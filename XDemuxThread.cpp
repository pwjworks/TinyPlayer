#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using namespace std;

void XDemuxThread::SetPause(bool isPause) {
	mux.lock();
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);
	mux.unlock();
}

void XDemuxThread::run()
{
	while (!isExit)
	{

		if (isPause) {
			msleep(5);
			continue;
		}
		mux.lock();
		if (!demux)
		{
			mux.unlock();
			msleep(5);
			continue;
		}


		//音视频同步
		if (vt && at)
		{
			pts = at->pts;
			vt->synpts = at->pts;
		}

		AVPacket* pkt = demux->Read();
		if (!pkt)
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		//判断数据是音频
		if (demux->IsAudio(pkt))
		{
			if (at)at->Push(pkt);
		}
		else //视频
		{
			if (vt)vt->Push(pkt);
		}

		mux.unlock();
		msleep(1);
	}
}


bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0')
		return false;

	mux.lock();


	//打开解封装
	bool re = demux->Open(url);
	if (!re)
	{
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open failed!" << endl;
	}
	//打开音频解码器和处理线程
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open failed!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	cout << "XDemuxThread::Open " << re << endl;
	return re;
}
// 关闭线程清理资源
void XDemuxThread::Close() {
	isExit = true;
	wait();
	if (vt)vt->Close();
	if (at) at->Close();
	mux.lock();
	delete vt;
	delete at;
	vt = nullptr;
	at = nullptr;
	mux.unlock();
}


//启动所有线程
void XDemuxThread::Start()
{
	mux.lock();
	if (!demux) demux = new XDemux();
	if (!vt) vt = new XVideoThread();
	if (!at) at = new XAudioThread();
	//启动当前线程
	QThread::start();
	if (vt)vt->start();
	if (at)at->start();
	mux.unlock();
	msleep(1);
}
XDemuxThread::XDemuxThread()
{
}


XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
}
