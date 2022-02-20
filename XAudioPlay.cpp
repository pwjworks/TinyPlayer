#include "XAudioPlay.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>
using namespace std;

class CXAudioPlay :public XAudioPlay {
public:
	virtual long long GetNoPlayMs() {
		mux.lock();
		if (!out) {
			mux.unlock();
			return 0;
		}
		long long pts = 0;
		// 还未播放的字节数
		double size = out->bufferSize() - out->bytesFree();
		// 一秒字节大小
		double secSize = sampleRate * (sampleSize / 8) * channels;
		if (secSize <= 0)
			pts = 0;
		else
			pts = (secSize / size) * 1000;
		mux.unlock();
		return pts;
	}
	virtual void Close() {
		std::lock_guard<std::mutex> lck(mux);
		if (out) {
			out->stop();
			delete out;
			out = nullptr;
		}
		if (io) {
			io->close();
			io = nullptr;
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