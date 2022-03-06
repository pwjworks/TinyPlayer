#include "XDecodeThread.h"
#include "XDecode.h"
#include "AVPacketRAII.h"

using namespace std;
void XDecodeThread::Close() {
	Clear();
	isExit = true;
	wait();
	decode->Close();
	lock_guard<mutex> lck(mux);
	delete decode;
	decode = nullptr;
}

void XDecodeThread::Clear() {
	lock_guard<mutex> lck(mux);
	decode->Clear();
	while (!packs.empty()) {
		packs.pop_front();
	}
}


shared_ptr<AVPacketRAII> XDecodeThread::Pop() {

	unique_lock<mutex> lck(mux);
	if (packs.empty()) {
		return nullptr;
	}
	shared_ptr<AVPacketRAII> pkt = packs.front();
	packs.pop_front();
	lck.unlock();
	msleep(1);
	return pkt;
}
void XDecodeThread::Push(shared_ptr<AVPacketRAII> pkt) {
	if (!pkt) return;
	while (!isExit) {
		unique_lock<mutex> lck(mux);
		if (packs.size() < maxList) {
			packs.emplace_back(pkt);
			break;
		}
		lck.unlock();
		msleep(1);
	}
}
XDecodeThread::XDecodeThread() {
	if (!decode) decode = new XDecode();
}
XDecodeThread::~XDecodeThread() {
	//等待线程退出
	isExit = true;
	wait();
}