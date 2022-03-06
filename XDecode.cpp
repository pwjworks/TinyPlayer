#include "XDecode.h"
#include "AVFrameRAII.h"
#include<iostream>
extern "C" {
#include <libavcodec/avcodec.h>
}
using namespace std;

void XFreePacket(AVPacket** pkt) {
	if (!pkt || !(*pkt)) return;
	av_packet_free(pkt);
}

void XFreeFrame(AVFrame** frame) {
	if (!frame || !(*frame)) return;
	av_frame_free(frame);
}

void XDecode::Close() {
	std::lock_guard<std::mutex> lck(mux_);
	if (codecContext) {
		avcodec_close(codecContext);
		avcodec_free_context(&codecContext);
	}
	pts = 0;
}
void XDecode::Clear() {
	std::lock_guard<std::mutex> lck(mux_);
	// 清理解码缓冲
	if (codecContext) avcodec_flush_buffers(codecContext);

}

bool XDecode::Open(AVCodecParameters* para) {
	if (!para) return false;
	// 打开视频解码器
	AVCodec* codec = avcodec_find_decoder(para->codec_id);
	if (!codec) {
		avcodec_parameters_free(&para);
		std::cout << "can't find the codec id " << para->codec_id << std::endl;
		return false;
	}
	std::cout << "codec found! " << para->codec_id << std::endl;

	// 加锁，保证codecContext访问互斥
	std::unique_lock<std::mutex> lck(mux_, std::try_to_lock);
	if (!lck.owns_lock()) return false;

	// 构建解码器上下文
	codecContext = avcodec_alloc_context3(codec);

	// 配置解码器上下文参数
	avcodec_parameters_to_context(codecContext, para);
	codecContext->thread_count = 8;

	int ret = avcodec_open2(codecContext, nullptr, nullptr);
	if (ret != 0) {
		avcodec_free_context(&codecContext);
		lck.unlock();
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		std::cout << "avcodec_open2 failed! :" << buf << std::endl;
		return false;
	}
	avcodec_parameters_free(&para);
	return true;
}

bool XDecode::Send(AVPacket* pkt) {
	if (!pkt || pkt->size <= 0 || !pkt->data) return false;
	{
		std::lock_guard<std::mutex> lck(mux_);
		if (!codecContext) return false;
		int ret = avcodec_send_packet(codecContext, pkt);
		// 释放空间
		av_packet_free(&pkt);
		if (ret != 0)return false;
	}
	return true;
}

shared_ptr<AVFrameRAII> XDecode::Recv() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!codecContext) return nullptr;
	shared_ptr<AVFrameRAII> frame = make_shared<AVFrameRAII>();
	int re = avcodec_receive_frame(codecContext, frame->get_frame());
	if (re != 0) {
		return nullptr;
	}
	// std::cout << "[" << frame->linesize[0] << "] " << std::flush;

	pts = frame->get_frame()->pts;

	return frame;
}

XDecode::XDecode() {};

XDecode::~XDecode() {};

