#pragma once
#include<mutex>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;

class XDemux
{
public:
	virtual bool Open(const char*);

	// �ռ���Ҫ�������ͷţ��ͷ�AVPacket����ռ䣬�Ϳռ�����av_packet_free
	virtual AVPacket* Read();

	virtual bool IsAudio(AVPacket* pkt);
	// ��ȡ��Ƶ����
	virtual AVCodecParameters* CopyVPara();


	// ��ȡ��Ƶ����
	virtual AVCodecParameters* CopyAPara();

	// seekλ��,pos=0.0~1.0
	virtual bool Seek(double pos);

	// ��ն�ȡ����
	virtual void Clear();

	virtual void Close();

	XDemux();
	virtual ~XDemux();

	// ý����ʱ��(����)
	int totalMs = 0;

	int width = 0;
	int height = 0;
	int sampleRate = 0;
	int channels = 0;
protected:
	std::mutex mux;
	// ���װ������
	AVFormatContext* ic{};
	int videoStream = 0;
	int audioStream = 1;

};

