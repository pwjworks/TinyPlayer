#include "XAudioThread.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include "AVFrameRAII.h"
#include <iostream>
using namespace std;

void XAudioThread::SetPause(bool isPause) {
	//amux.lock();
	this->isPause = isPause;
	if (ap) ap->SetPause(isPause);
	//amux.unlock();
}
void XAudioThread::Clear() {
	XDecodeThread::Clear();
	lock_guard<mutex> lck(amux);
	if (ap)ap->Clear();
}
void XAudioThread::Close() {
	XDecodeThread::Close();
	unique_lock<mutex> lck(amux);
	if (res) {
		res->Close();
		res.reset();
	}
	if (ap) {
		ap->Close();
		ap = nullptr;
	}
}

bool XAudioThread::Open(AVCodecParameters* para, int sampleRate, int channels)
{
	if (!para)return false;
	unique_lock<mutex> lck(amux);
	pts = 0;

	bool re = true;
	if (!res->Open(para, false))
	{
		cout << "XResample open failed!" << endl;
		re = false;
	}
	ap->sampleRate = sampleRate;
	ap->channels = channels;
	if (!ap->Open())
	{
		re = false;
		cout << "XAudioPlay open failed!" << endl;
	}
	if (!decode->Open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		re = false;
	}
	lck.unlock();
	cout << "XAudioThread::Open :" << re << endl;
	return re;
}

void XAudioThread::run()
{
	unsigned char* pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit)
	{
		unique_lock<mutex> lck(amux);

		if (isPause) {
			lck.unlock();
			msleep(5);
			continue;
		}

		AVPacket* pkt = Pop();
		bool re = decode->Send(pkt);
		if (!re)
		{
			lck.unlock();
			msleep(1);
			continue;
		}
		//һ��send ���recv
		while (!isExit)
		{
			shared_ptr<AVFrameRAII> frame = decode->Recv();
			if (!frame) break;

			//��ȥ������δ���ŵ�ʱ��
			pts = decode->pts - ap->GetNoPlayMs();

			//cout << "audio pts = " << pts << endl;

			//�ز��� 
			int size = res->Resample(frame, pcm);

			//������Ƶ
			while (!isExit)
			{
				if (size <= 0)break;
				//����δ���꣬�ռ䲻��
				if (ap->GetFree() < size || isPause)
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm, size);
				break;
			}
		}
		lck.unlock();
	}
	delete pcm;
}

XAudioThread::XAudioThread() :res(make_shared<XResample>())
{
	if (!ap) ap = XAudioPlay::Get();
}


XAudioThread::~XAudioThread()
{
}
