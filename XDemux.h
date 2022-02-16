#pragma once
#include<mutex>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;

class XDemux
{
public:
	virtual bool Open(const char*);

	// 空间需要调用者释放，释放AVPacket对象空间，和空间数据av_packet_free
	virtual AVPacket* Read();

	virtual bool IsAudio(AVPacket* pkt);
	// 获取视频参数
	virtual AVCodecParameters* CopyVPara();


	// 获取音频参数
	virtual AVCodecParameters* CopyAPara();

	// seek位置,pos=0.0~1.0
	virtual bool Seek(double pos);

	// 清空读取缓存
	virtual void Clear();

	virtual void Close();

	XDemux();
	virtual ~XDemux();

	// 媒体总时长(毫秒)
	int totalMs = 0;
protected:
	std::mutex mux_;
	// 解封装上下文
	AVFormatContext* ic{};
	int videoStream = 0;
	int audioStream = 1;

};

