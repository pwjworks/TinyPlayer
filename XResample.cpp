#include "XResample.h"
#include "AVFrameRAII.h"
extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}
#pragma comment(lib,"swresample.lib")
#include <iostream>
using namespace std;

void XResample::Close()
{
	lock_guard<mutex> lck(mux);
	if (actx)
		swr_free(&actx);
}

//����������������һ�³��˲�����ʽ�����ΪS16
bool XResample::Open(AVCodecParameters* para, bool isClearPara)
{
	if (!para)return false;
	mux.lock();
	//��Ƶ�ز��� �����ĳ�ʼ��
	//���actxΪNULL�����ռ�
	actx = swr_alloc_set_opts(actx,
		av_get_default_channel_layout(2),	//�����ʽ
		(AVSampleFormat)outFormat,			//���������ʽ 1 AV_SAMPLE_FMT_S16
		para->sample_rate,					//���������
		av_get_default_channel_layout(para->channels),//�����ʽ
		(AVSampleFormat)para->format,
		para->sample_rate,
		0, 0
	);
	if (isClearPara)
		avcodec_parameters_free(&para);
	int re = swr_init(actx);
	mux.unlock();
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "swr_init  failed! :" << buf << endl;
		return false;
	}

	return true;
}

//�����ز������С,���ܳɹ�����ͷ�indata�ռ�
int XResample::Resample(shared_ptr<AVFrameRAII> indata, unsigned char* d)
{
	// �ݴ�
	if (!indata) return 0;
	if (!d)
	{
		return 0;
	}
	uint8_t* data[2] = { 0 };
	data[0] = d;
	int re = swr_convert(actx,
		data, indata->get_frame()->nb_samples,		//���
		(const uint8_t**)indata->get_frame()->data, indata->get_frame()->nb_samples	//����
	);
	if (re <= 0)return re;
	int outSize = re * indata->get_frame()->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);

	return outSize;
}
XResample::XResample()
{
}


XResample::~XResample()
{
}
