#pragma once
#include <QThread>
#include <mutex>
#include <list>
struct AVCodecParameters;
class XDecode;
class XAudioPlay;
class XResample;
class AVPacket;
class XAudioThread :public QThread
{
public:

	//打开，不管成功与否都清理
	virtual bool Open(AVCodecParameters* para, int sampleRate, int channels);
	virtual void Push(AVPacket* pkt);
	void run();
	XAudioThread();
	virtual ~XAudioThread();

	//最大队列
	int maxList = 100;
	bool isExit = false;
protected:

	std::list <AVPacket*> packs;
	std::mutex mux;
	XDecode* decode = 0;
	XAudioPlay* ap = 0;
	XResample* res = 0;

};