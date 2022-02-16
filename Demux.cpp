#include "Demux.h"
extern "C" {
	#include"libavformat/avformat.h"
}
#pragma comment(lib,"avformat.lib")
bool Demux::Open(const char* url) {
	
	return true;
}

Demux::Demux() {
	static bool isFirst = true;
	// ��ֻ֤��ʼ��һ��
	static std::mutex dmux_;
	dmux_.lock();
	if (isFirst) {
		av_register_all();

		avformat_network_init();
		isFirst = false;
	}
	dmux_.unlock();
}