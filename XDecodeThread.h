#pragma once
#include <QThread>
#include <mutex>
#include <list>
class XDecode;
struct AVPacket;



class XDecodeThread : public QThread
{
public:
	XDecodeThread();
	virtual ~XDecodeThread();
	virtual void Push(AVPacket* pkt);

	// �������
	virtual void Clear();
	// ������Դ��ֹͣ�߳�
	virtual void Close();

	// ȡ��һ֡���ݣ�����ջ�����û�з���NULL
	virtual AVPacket* Pop();
	//������
	int maxList = 100;
	bool isExit = false;
protected:
	std::list <AVPacket*> packs;
	std::mutex mux;
	XDecode* decode = 0;
};

