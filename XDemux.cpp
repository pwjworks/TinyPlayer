#include "XDemux.h"
#include <iostream>
extern "C" {
#include"libavformat/avformat.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

static double r2d(AVRational r) {
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
bool XDemux::Open(const char* url) {
	Close();
	// 注册解码器
	AVDictionary* opts = nullptr;
	// 设置rtsp流和tcp协议
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	// 网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	// 加锁，避免争用
	std::lock_guard<std::mutex> lck(mux_);
	// 打开输入流
	int re = avformat_open_input(&ic,
		url,
		0,  // 0表示自动选择解封装器
		&opts
	);
	if (re != 0) {
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		std::cout << "open " << url << " failed! :" << buf << std::endl;
		return false;
	}
	std::cout << "open " << url << " success!" << std::endl;

	// 获取流信息
	re = avformat_find_stream_info(ic, 0);

	// 计算总时长，单位毫秒
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	std::cout << "totalMs = " << totalMs << std::endl;

	// 打印视频流详细信息
	av_dump_format(ic, 0, url, 0);

	// 音视频索引，读取时区分音视频
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);;
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);;

	// 获取视频流
	AVStream* vs = ic->streams[videoStream];
	// 获取音频流
	AVStream* as = ic->streams[audioStream];

	std::cout << videoStream << "视频信息" << std::endl;
	std::cout << "codec_id = " << vs->codecpar->codec_id << std::endl;
	std::cout << "format = " << vs->codecpar->format << std::endl;
	std::cout << "width=" << vs->codecpar->width << std::endl;
	std::cout << "height=" << vs->codecpar->height << std::endl;
	// 帧率fps分数转换
	std::cout << "video fps = " << r2d(vs->avg_frame_rate) << std::endl;


	std::cout << audioStream << "音频信息" << std::endl;
	std::cout << "codec_id = " << as->codecpar->codec_id << std::endl;
	std::cout << "format = " << as->codecpar->format << std::endl;
	std::cout << "sample_rate = " << as->codecpar->sample_rate << std::endl;
	std::cout << "channels = " << as->codecpar->channels << std::endl;
	std::cout << "frame_size = " << as->codecpar->frame_size << std::endl;
	return true;
}

void XDemux::Clear() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!ic) return;
	avformat_flush(ic);
};

void XDemux::Close() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!ic) return;
	avformat_close_input(&ic);
	// 重置媒体总时长
	totalMs = 0;
}

bool XDemux::Seek(double pos) {
	int re;
	{
		std::lock_guard<std::mutex> lck(mux_);
		if (!ic) return false;
		//清理读取缓冲
		avformat_flush(ic);

		// 计算跳转位置
		int64_t seekpos = 0;
		seekpos = ic->streams[videoStream]->duration * pos;

		re = av_seek_frame(ic, videoStream, seekpos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	}
	if (re < 0) return false;
	return true;
};

/**
	@brief 复制获得音频参数
**/
AVCodecParameters* XDemux::CopyVPara() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!ic) {
		return nullptr;
	}
	AVCodecParameters* pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	return pa;
};


/**
	@brief 复制获得音频参数
**/
AVCodecParameters* XDemux::CopyAPara() {
	std::lock_guard<std::mutex> lck(mux_);
	if (!ic) {
		return nullptr;
	}
	AVCodecParameters* pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	return pa;
};


AVPacket* XDemux::Read() {
	AVPacket* pkt = nullptr;
	{
		std::lock_guard<std::mutex> lck(mux_);
		if (!ic) {
			return nullptr;
		}
		pkt = av_packet_alloc();
		int ret = av_read_frame(ic, pkt);
		if (ret != 0) {
			av_packet_free(&pkt);
			return nullptr;
		}
	}

	// pts转换为毫秒
	pkt->pts = pkt->pts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));


	std::cout << pkt->pts << std::endl;
	return pkt;
}

bool XDemux::IsAudio(AVPacket* pkt) {
	if (!pkt) return false;
	if (pkt->stream_index == videoStream) return false;
	return true;
}

XDemux::XDemux() {
	static bool isFirst = true;
	// 保证只初始化一次
	static std::mutex dmux_;
	{
		std::lock_guard<std::mutex> lck(dmux_);
		if (isFirst) {
			av_register_all();

			avformat_network_init();
			isFirst = false;
		}
	}

}

XDemux::~XDemux() {}