#include "XVideoThread.h"
#include "XDecode.h"
#include <iostream>
using namespace std;


void XVideoThread::SetPause(bool isPause) {
	//vmux.lock();
	this->isPause = isPause;
	//vmux.unlock();
}

bool XVideoThread::RepaintPts(AVPacket* pkt, long long seekpts) {
	lock_guard<mutex> lck(vmux);
	bool re = decode->Send(pkt);
	if (!re) {
		return true; // ��������
	}
	AVFrame* frame = decode->Recv();
	if (!frame) {
		return false;
	}
	if (decode->pts >= seekpts) {
		if (call) call->Repaint(frame);
		return true;
	}
	XFreeFrame(&frame);
	return false;

}
//�򿪣����ܳɹ��������
bool XVideoThread::Open(AVCodecParameters* para, IVideoCall* call, int width, int height)
{
	if (!para)return false;

	Clear();
	unique_lock<mutex> lck(vmux);
	synpts = 0;
	//��ʼ����ʾ����
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}

	//�򿪽�����
	if (!decode) decode = new XDecode();
	lck.unlock();
	int re = true;
	if (!decode->Open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		re = false;
	}

	cout << "XAudioThread::Open :" << re << endl;
	return re;
}

void XVideoThread::run()
{
	while (!isExit)
	{
		unique_lock<mutex> lck(vmux);
		if (isPause) {
			lck.unlock();
			msleep(5);
			continue;
		}
		//����Ƶͬ��
		if (synpts > 0 && synpts < decode->pts) {
			lck.unlock();
			msleep(1);
			continue;
		}
		AVPacket* pkt = Pop();

		////û������
		//if (packs.empty() || !decode) {
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}



		//AVPacket* pkt = packs.front();
		//packs.pop_front();
		bool re = decode->Send(pkt);
		if (!re) {
			lck.unlock();
			msleep(1);
			continue;
		}
		//һ��send ���recv
		while (!isExit) {
			AVFrame* frame = decode->Recv();
			if (!frame) break;
			//��ʾ��Ƶ
			if (call)
			{
				call->Repaint(frame);
			}

		}
		lck.unlock();
	}
}

XVideoThread::XVideoThread()
{
}


XVideoThread::~XVideoThread()
{

}
