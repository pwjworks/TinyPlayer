#pragma once

///�������ʾ��Ƶ
struct AVCodecParameters;
class XDecode;
class AVPacketRAII;
#include <list>
#include <mutex>
#include <QThread>
#include "IVideoCall.h"
#include "XDecodeThread.h"
class XVideoThread :public XDecodeThread
{
public:
	// ����pts��������յ��Ľ�������pts>=seekpts��return true����ʾ���档
	virtual bool RepaintPts(std::shared_ptr<AVPacketRAII>, long long seekpts);
	//�򿪣����ܳɹ��������
	virtual bool Open(AVCodecParameters* para, IVideoCall* call, int width, int height);

	void run();

	XVideoThread();
	virtual ~XVideoThread();
	// ͬ��ʱ�䣬���ⲿ����
	long long synpts = 0;

	void SetPause(bool isPause);
	bool isPause = false;


protected:
	IVideoCall* call = 0;
	std::mutex vmux;

};
