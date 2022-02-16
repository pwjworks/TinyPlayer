#include "XDecode.h"
#include<iostream>
extern "C" {
#include <libavcodec/avcodec.h>
}


void XDecode::Close() {
	std::lock_guard<std::mutex> lck(mux_);
	if (codecContext) {
		avcodec_close(codecContext);
		avcodec_free_context(&codecContext);
	}
}
void XDecode::Clear() {
	std::lock_guard<std::mutex> lck(mux_);
	// �������뻺��
	if (codecContext) avcodec_flush_buffers(codecContext);

}

bool XDecode::Open(AVCodecParameters* para) {
	if (!para) return false;
	// ����Ƶ������
	AVCodec* codec = avcodec_find_decoder(para->codec_id);
	if (!codec) {
		avcodec_parameters_free(&para);
		std::cout << "can't find the codec id " << para->codec_id << std::endl;
		return false;
	}
	std::cout << "codec found! " << para->codec_id << std::endl;

	// ��������֤codecContext���ʻ���
	std::unique_lock<std::mutex> lck(mux_, std::try_to_lock);
	if (!lck.owns_lock()) return false;

	// ����������������
	codecContext = avcodec_alloc_context3(codec);

	// ���ý����������Ĳ���
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
		// �ͷſռ�
		av_packet_free(&pkt);
		if (ret != 0)return false;
	}
	return true;
}

AVFrame* XDecode::Recv() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!codecContext) return nullptr;
	AVFrame* frame = av_frame_alloc();
	int re = avcodec_receive_frame(codecContext, frame);
	if (re != 0) {
		av_frame_free(&frame);
		return nullptr;
	}
	std::cout << "[" << frame->linesize[0] << "] " << std::flush;
	return frame;
}

XDecode::XDecode() {};

XDecode::~XDecode() {};
