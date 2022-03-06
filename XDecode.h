#pragma once
#include<mutex>
#include <memory>

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class AVPacketRAII;
class AVFrameRAII;

class XDecode {
public:
	bool isAudio = false;
	long long pts = 0;
	// �򿪽����������ܳɹ�����ͷ�para�ռ䡣
	virtual bool Open(AVCodecParameters* para);

	// ���͵������̣߳����ܳɹ�����ͷ�pkt�ռ䣨�����ý�����ݣ�
	virtual bool Send(std::shared_ptr<AVPacketRAII> pkt);

	//��ȡ�������ݣ�һ��send������Ҫ���Recv����ȡ�����е�����Send NULL��Recv���
	//ÿ�θ���һ�ݣ��ɵ������ͷ� av_frame_free
	virtual std::shared_ptr<AVFrameRAII> Recv();

	virtual void Close();
	virtual void Clear();
	XDecode();
	~XDecode();
private:
	AVCodecContext* codecContext = 0;
	std::mutex mux_;
};

