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
	// 当前音频播放的pts
	long long pts;
	//打开，不管成功与否都清理
	virtual bool Open(AVCodecParameters* para, int sampleRate, int channels);

	//停止线程，清理资源
	virtual void Stop();
	void run();
	XAudioThread();
	virtual ~XAudioThread();

protected:
	std::mutex amux;
	XAudioPlay* ap = 0;
	XResample* res = 0;

};