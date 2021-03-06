#pragma once

#include <mutex>
#include <memory>

struct AVCodecParameters;
struct AVFrame;
struct SwrContext;

class AVFrameRAII;

class XResample
{
public:

	//输出参数和输入参数一致除了采样格式，输出为S16 ,会释放para
	virtual bool Open(AVCodecParameters* par, bool isClearPara = false);
	virtual void Close();

	//返回重采样后大小,不管成功与否都释放indata空间
	virtual int Resample(std::shared_ptr<AVFrameRAII> indata, unsigned char* data);
	XResample();
	~XResample();

	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
protected:
	std::mutex mux;
	SwrContext* actx = 0;
};

