#include "XDemux.h"
#include <iostream>
using namespace std;
extern "C" {
#include "libavformat/avformat.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

bool XDemux::Open(const char* url)
{
	Close();
	//��������
	AVDictionary* opts = NULL;
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	std::unique_lock<mutex> lck(mux);
	int re = avformat_open_input(
		&ic,
		url,
		0,  // 0��ʾ�Զ�ѡ������
		&opts //�������ã�����rtsp����ʱʱ��
	);
	if (re != 0)
	{
		lck.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "open " << url << " success! " << endl;

	//��ȡ����Ϣ 
	re = avformat_find_stream_info(ic, 0);

	//��ʱ�� ����
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	//��ӡ��Ƶ����ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);


	//��ȡ��Ƶ��
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream* as = ic->streams[videoStream];
	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "=======================================================" << endl;
	cout << videoStream << "��Ƶ��Ϣ" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;
	//֡�� fps ����ת��
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	cout << "=======================================================" << endl;
	cout << audioStream << "��Ƶ��Ϣ" << endl;
	//��ȡ��Ƶ��
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = ic->streams[audioStream];
	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	//AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	//һ֡���ݣ��� ��ͨ�������� 
	cout << "frame_size = " << as->codecpar->frame_size << endl;
	//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size
	return true;
}
//��ն�ȡ����
void XDemux::Clear()
{
	std::unique_lock<mutex> lck(mux);
	if (!ic)
	{
		return;
	}
	//�����ȡ����
	avformat_flush(ic);
}
void XDemux::Close()
{
	std::unique_lock<mutex> lck(mux);
	if (!ic)
	{
		return;
	}
	avformat_close_input(&ic);
	//ý����ʱ�������룩
	totalMs = 0;
}

//seek λ�� pos 0.0 ~1.0
bool XDemux::Seek(double pos)
{
	std::unique_lock<mutex> lck(mux);
	if (!ic)
	{
		return false;
	}
	//�����ȡ����
	avformat_flush(ic);

	long long seekPos = 0;
	seekPos = ic->streams[videoStream]->duration * pos;
	int re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	if (re < 0) return false;
	return true;
}
//��ȡ��Ƶ����  ���صĿռ���Ҫ����  avcodec_parameters_free
AVCodecParameters* XDemux::CopyVPara()
{
	std::unique_lock<mutex> lck(mux);
	if (!ic) {
		return nullptr;
	}
	AVCodecParameters* pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	return pa;
}

//��ȡ��Ƶ����  ���صĿռ���Ҫ���� avcodec_parameters_free
AVCodecParameters* XDemux::CopyAPara()
{
	std::unique_lock<mutex> lck(mux);
	if (!ic) {
		return nullptr;
	}
	AVCodecParameters* pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	return pa;
}

AVPacket* XDemux::ReadVideo() {
	AVPacket* pkt = nullptr;
	// ��ֹ����
	for (int i = 0; i < 20; i++) {
		pkt = Read();
		if (!pkt)break;
		if (pkt->stream_index == videoStream)break;
		av_packet_free(&pkt);
	}
	return pkt;
}

bool XDemux::IsAudio(AVPacket* pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == videoStream)
		return false;
	return true;

}
//�ռ���Ҫ�������ͷ� ���ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
AVPacket* XDemux::Read()
{
	std::unique_lock<mutex> lck(mux);
	if (!ic) //�ݴ�
	{
		return 0;
	}
	AVPacket* pkt = av_packet_alloc();
	//��ȡһ֡��������ռ�
	int re = av_read_frame(ic, pkt);
	if (re != 0)
	{
		lck.unlock();
		av_packet_free(&pkt);
		return 0;
	}
	//ptsת��Ϊ����
	pkt->pts = pkt->pts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	cout << pkt->pts << " " << flush;
	return pkt;

}
XDemux::XDemux()
{
	static bool isFirst = true;
	static std::mutex dmux;
	std::unique_lock<mutex> lck(dmux);
	if (isFirst)
	{
		//��ʼ����װ��
		av_register_all();

		//��ʼ������� �����Դ�rtsp rtmp http Э�����ý����Ƶ��
		avformat_network_init();
		isFirst = false;
	}
}


XDemux::~XDemux()
{
}