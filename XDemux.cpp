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
	// ע�������
	AVDictionary* opts = nullptr;
	// ����rtsp����tcpЭ��
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	// ������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	// ��������������
	std::lock_guard<std::mutex> lck(mux_);
	// ��������
	int re = avformat_open_input(&ic,
		url,
		0,  // 0��ʾ�Զ�ѡ����װ��
		&opts
	);
	if (re != 0) {
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		std::cout << "open " << url << " failed! :" << buf << std::endl;
		return false;
	}
	std::cout << "open " << url << " success!" << std::endl;

	// ��ȡ����Ϣ
	re = avformat_find_stream_info(ic, 0);

	// ������ʱ������λ����
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	std::cout << "totalMs = " << totalMs << std::endl;

	// ��ӡ��Ƶ����ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);

	// ����Ƶ��������ȡʱ��������Ƶ
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);;
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);;

	// ��ȡ��Ƶ��
	AVStream* vs = ic->streams[videoStream];
	// ��ȡ��Ƶ��
	AVStream* as = ic->streams[audioStream];

	std::cout << videoStream << "��Ƶ��Ϣ" << std::endl;
	std::cout << "codec_id = " << vs->codecpar->codec_id << std::endl;
	std::cout << "format = " << vs->codecpar->format << std::endl;
	std::cout << "width=" << vs->codecpar->width << std::endl;
	std::cout << "height=" << vs->codecpar->height << std::endl;
	// ֡��fps����ת��
	std::cout << "video fps = " << r2d(vs->avg_frame_rate) << std::endl;


	std::cout << audioStream << "��Ƶ��Ϣ" << std::endl;
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
	// ����ý����ʱ��
	totalMs = 0;
}

bool XDemux::Seek(double pos) {
	int re;
	{
		std::lock_guard<std::mutex> lck(mux_);
		if (!ic) return false;
		//�����ȡ����
		avformat_flush(ic);

		// ������תλ��
		int64_t seekpos = 0;
		seekpos = ic->streams[videoStream]->duration * pos;

		re = av_seek_frame(ic, videoStream, seekpos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	}
	if (re < 0) return false;
	return true;
};

/**
	@brief ���ƻ����Ƶ����
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
	@brief ���ƻ����Ƶ����
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

	// ptsת��Ϊ����
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
	// ��ֻ֤��ʼ��һ��
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