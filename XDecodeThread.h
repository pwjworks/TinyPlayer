#pragma once
#include <QThread>
#include <mutex>
#include <list>
class XDecode;

class AVPacketRAII;

class XDecodeThread : public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	virtual void Push(std::shared_ptr<AVPacketRAII> pkt);

	// �������
	virtual void Clear();
	// ������Դ��ֹͣ�߳�
	virtual void Close();

	// ȡ��һ֡���ݣ�����ջ�����û�з���NULL
	virtual std::shared_ptr<AVPacketRAII> Pop();
	//������
	int maxList = 200;
	bool isExit = false;


protected:
	XDecode* decode = 0;
	std::list <std::shared_ptr<AVPacketRAII>> packs;
	std::mutex mux;

};

