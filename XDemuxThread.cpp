#include "XDemuxThread.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
extern "C" {
#include<libavformat/avformat.h>
}
#include "XDecode.h"
using namespace std;

void XDemuxThread::Clear() {
	lock_guard<mutex> lck(mux);
	if (demux) demux->Clear();
	if (vt) vt->Clear();
	if (at) at->Clear();
}

void XDemuxThread::Seek(double pos) {
	// 清理缓存
	Clear();
	unique_lock<mutex> lck(mux);
	bool status = this->isPause;
	lck.unlock();
	SetPause(true);

	{
		lock_guard<mutex> lck(mux);
		if (demux) demux->Seek(pos);
		// 实际要显示的位置pts
		long long seekPts = pos * demux->totalMs;
		while (!isExit) {
			shared_ptr<AVPacketRAII> pkt = demux->ReadVideo();
			if (!pkt) break;
			// 如果解码到seekPts
			if (vt->RepaintPts(pkt, seekPts)) {
				this->pts = seekPts;
				break;
			}
		}
	}

	// seek时非暂停状态
	if (!status)
		SetPause(false);
}

void XDemuxThread::SetPause(bool isPause) {
	lock_guard<mutex> lck(mux);
	this->isPause = isPause;
	if (at) at->SetPause(isPause);
	if (vt) vt->SetPause(isPause);
}

void XDemuxThread::run()
{
	while (!isExit)
	{

		std::unique_lock<mutex> lck(mux);
		if (this->isPause) {
			lck.unlock();
			msleep(5);
			continue;
		}

		if (!demux) {
			lck.unlock();
			msleep(5);
			continue;
		}

		//音视频同步
		if (vt && at) {
			pts = at->pts;
			vt->synpts = at->pts;
		}

		// 解封装音视频
		shared_ptr<AVPacketRAII> pkt = demux->Read();
		if (!pkt) {
			lck.unlock();
			msleep(5);
			continue;
		}
		//判断数据是音频
		if (demux->IsAudio(pkt)) {
			if (at)at->Push(pkt);
		}
		else {
			if (vt)vt->Push(pkt);//视频
		}
		lck.unlock();
		msleep(1);
	}
}


bool XDemuxThread::Open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0')
		return false;

	lock_guard<mutex> lck(mux);

	//打开解封装
	bool re = demux->Open(url);
	if (!re) {
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	//打开视频解码器和处理线程
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height)) {
		re = false;
		cout << "vt->Open failed!" << endl;
	}
	//打开音频解码器和处理线程
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;
	}
	totalMs = demux->totalMs;
	cout << "XDemuxThread::Open " << re << endl;
	return re;
}
// 关闭线程清理资源
void XDemuxThread::Close() {
	isExit = true;
	wait();
	if (vt)vt->Close();
	if (at) at->Close();
	lock_guard<mutex> lck(mux);
	vt.reset();
	at.reset();

}


//启动所有线程
void XDemuxThread::Start()
{
	//启动当前线程
	QThread::start();
	if (vt)vt->start();
	if (at)at->start();
	msleep(1);
}
XDemuxThread::XDemuxThread() :demux(make_shared<XDemux>()),
vt(make_shared<XVideoThread>()),
at(make_shared<XAudioThread>())
{
}


XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
}
