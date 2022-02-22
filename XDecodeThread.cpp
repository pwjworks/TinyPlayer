#include "XDecodeThread.h"
#include "XDecode.h"

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
		AVPacket* pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
}


AVPacket* XDecodeThread::Pop() {

	unique_lock<mutex> lck(mux);
	if (packs.empty()) {
		return nullptr;
	}
	AVPacket* pkt = packs.front();
	packs.pop_front();
	lck.unlock();
	msleep(1);
	return pkt;
}
void XDecodeThread::Push(AVPacket* pkt) {
	if (!pkt) return;
	while (!isExit) {
		unique_lock<mutex> lck(mux);
		if (packs.size() < maxList) {
			packs.emplace_back(pkt);
			lck.unlock();
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