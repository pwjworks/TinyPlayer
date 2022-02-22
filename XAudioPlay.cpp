#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>
using namespace std;

class CXAudioPlay :public XAudioPlay {
public:
	virtual long long GetNoPlayMs() {
		std::lock_guard<std::mutex> lck(mux);
		if (!out) {
			return 0;
		}
		long long pts = 0;
		// ��δ���ŵ��ֽ���
		double size = out->bufferSize() - out->bytesFree();
		// һ���ֽڴ�С
		double secSize = sampleRate * (sampleSize / 8) * channels;
		if (secSize <= 0)
			pts = 0;
		else
			pts = (size / secSize) * 1000;
		return pts;
	}
	virtual void Clear() {
		std::lock_guard<std::mutex> lck(mux);
		if (io) {
			io->reset();
		}
	}
	virtual void Close() {
		std::lock_guard<std::mutex> lck(mux);
		if (io) {
			io->close();
			io = nullptr;
		}
		if (out) {
			out->stop();
			delete out;
			out = nullptr;
		}

	}
	virtual void SetPause(bool isPause) {
		std::lock_guard<std::mutex> lck(mux);
		if (!out) {
			return;
		}
		if (isPause) {
			out->suspend();
		}
		else {
			out->resume();
		}
	}

	virtual bool Open() {
		Close();
		QAudioFormat fmt;
		fmt.setSampleRate(44100);
		fmt.setSampleSize(16);
		fmt.setChannelCount(2);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		{
			std::lock_guard<std::mutex> lck(mux);
			out = new QAudioOutput(fmt);
			io = out->start();
		}
		if (io)
			return true;
		return false;
	}
	virtual bool Write(const unsigned char* data, int datasize) {
		if (!data || datasize <= 0)return false;
		lock_guard<mutex> lck(mux);
		if (!out || !io)	return false;
		io->write((char*)data, datasize);

		return true;
	}
	virtual int GetFree() {
		lock_guard<mutex> lck(mux);
		if (!out) {
			return 0;
		}
		int free = out->bytesFree();
		return free;
	}
	QAudioOutput* out;
	QIODevice* io;
	mutex mux;
};


XAudioPlay* XAudioPlay::Get() {
	static CXAudioPlay play;
	return &play;
}

XAudioPlay::XAudioPlay() {

}

XAudioPlay::~XAudioPlay() {

}