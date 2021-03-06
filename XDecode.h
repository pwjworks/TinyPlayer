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
	// 打开解码器，不管成功与否都释放para空间。
	virtual bool Open(AVCodecParameters* para);

	// 发送到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
	virtual bool Send(std::shared_ptr<AVPacketRAII> pkt);

	//获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
	//每次复制一份，由调用者释放 av_frame_free
	virtual std::shared_ptr<AVFrameRAII> Recv();

	virtual void Close();
	virtual void Clear();
	XDecode();
	~XDecode();
private:
	AVCodecContext* codecContext = 0;
	std::mutex mux_;
};

