#pragma once
#include <QThread>
#include <mutex>
#include <list>
#include "XDecodeThread.h"
struct AVCodecParameters;
class XAudioPlay;
class XResample;
class XAudioThread :public XDecodeThread
{
public:
	// ��ǰ��Ƶ���ŵ�pts
	long long pts;
	//�򿪣����ܳɹ��������
	virtual bool Open(AVCodecParameters* para, int sampleRate, int channels);

	//ֹͣ�̣߳�������Դ
	virtual void Stop();
	void run();
	XAudioThread();
	virtual ~XAudioThread();

protected:
	std::mutex amux;
	XAudioPlay* ap = 0;
	XResample* res = 0;

};