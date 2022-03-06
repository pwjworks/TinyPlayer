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
	// ������
	Clear();
	unique_lock<mutex> lck(mux);
	bool status = this->isPause;
	lck.unlock();
	SetPause(true);

	{
		lock_guard<mutex> lck(mux);
		if (demux) demux->Seek(pos);
		// ʵ��Ҫ��ʾ��λ��pts
		long long seekPts = pos * demux->totalMs;
		while (!isExit) {
			shared_ptr<AVPacketRAII> pkt = demux->ReadVideo();
			if (!pkt) break;
			// ������뵽seekPts
			if (vt->RepaintPts(pkt, seekPts)) {
				this->pts = seekPts;
				break;
			}
		}
	}

	// seekʱ����ͣ״̬
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

		//����Ƶͬ��
		if (vt && at) {
			pts = at->pts;
			vt->synpts = at->pts;
		}

		// ���װ����Ƶ
		shared_ptr<AVPacketRAII> pkt = demux->Read();
		if (!pkt) {
			lck.unlock();
			msleep(5);
			continue;
		}
		//�ж���������Ƶ
		if (demux->IsAudio(pkt)) {
			if (at)at->Push(pkt);
		}
		else {
			if (vt)vt->Push(pkt);//��Ƶ
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

	//�򿪽��װ
	bool re = demux->Open(url);
	if (!re) {
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	//����Ƶ�������ʹ����߳�
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height)) {
		re = false;
		cout << "vt->Open failed!" << endl;
	}
	//����Ƶ�������ʹ����߳�
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels)) {
		re = false;
		cout << "at->Open failed!" << endl;
	}
	totalMs = demux->totalMs;
	cout << "XDemuxThread::Open " << re << endl;
	return re;
}
// �ر��߳�������Դ
void XDemuxThread::Close() {
	isExit = true;
	wait();
	if (vt)vt->Close();
	if (at) at->Close();
	lock_guard<mutex> lck(mux);
	vt.reset();
	at.reset();

}


//���������߳�
void XDemuxThread::Start()
{
	//������ǰ�߳�
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
