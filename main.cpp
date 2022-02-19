#include "tinyplayer.h"
#include "XDemux.h"
#include "XResample.h"
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XAudioThread.h"
#include "XVideoThread.h"
#include "XDemuxThread.h"
#include <QThread>
#include <QtWidgets/QApplication>
#include <iostream>


extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace std;

class TestThread :public QThread {
public:
	void Init() {
		//cout << "demux.open = " << demux.Open("test.mp4");

		cout << "demux.Open = " << demux.Open("test.mp4");
		cout << "CopyVPara: " << demux.CopyVPara() << endl;
		cout << "CopyAPara: " << demux.CopyAPara() << endl;
		cout << "seed = " << demux.Seek(0.5) << endl;

		//cout << "vdecoder.Open =" << vdecoder.Open(demux.CopyVPara()) << endl;
		//cout << "adecoder.Open =" << adecoder.Open(demux.CopyAPara()) << endl;
		//resample.Open(demux.CopyAPara(), true);
		//XAudioPlay::Get()->channels = demux.channels;
		//XAudioPlay::Get()->sampleRate = demux.sampleRate;
		//cout << "XAudioPlay::Open()" << XAudioPlay::Get()->Open() << endl;
		//cout << "=================" << endl;
		cout << at.Open(demux.CopyAPara(), demux.sampleRate, demux.channels);
		vt.Open(demux.CopyVPara(), video, demux.width, demux.height);
		at.start();
		vt.start();
	}
	unsigned char* pcm = new unsigned char[1024 * 1024];
	void run() {
		for (;;) {
			AVPacket* pkt = demux.Read();
			if (demux.IsAudio(pkt)) {
				at.Push(pkt);
				//adecoder.Send(pkt);
				//AVFrame* frame = adecoder.Recv();
				//int len = resample.Resample(frame, pcm);
				//while (len > 0) {
				//	if (XAudioPlay::Get()->GetFree() >= len) {
				//		XAudioPlay::Get()->Write(pcm, len);
				//		break;
				//	}
				//msleep(1);
				//}


				//cout << "Resample:" << len << " ";
				//cout << "Audio: " << frame << endl;
				//av_frame_free(&frame);
			}
			else {
				vt.Push(pkt);
				//av_frame_free(&frame);
			}
			if (!pkt) break;
		}
	}
	XDemux demux;
	XVideoWidget* video = 0;
	XAudioThread at;
	XVideoThread vt;
};

int main(int argc, char* argv[])
{
	//XDemux demux;
	//cout << "demux.open = " << demux.Open("test.mp4");

	//demux.Clear();
	//demux.Close();
	//cout << "demux.Open = " << demux.Open("test.mp4");
	//cout << "CopyVPara: " << demux.CopyVPara() << endl;
	//cout << "CopyAPara: " << demux.CopyAPara() << endl;
	//cout << "seed = " << demux.Seek(0.5) << endl;

	//XDecode vdecoder;
	//cout << "vdecoder.Open =" << vdecoder.Open(demux.CopyVPara()) << endl;
	////vdecoder.Clear();
	////vdecoder.Close();
	//XDecode adecoder;
	//cout << "adecoder.Open =" << adecoder.Open(demux.CopyAPara()) << endl;
	////for (;;) {
	//	AVPacket* pkt = demux.Read();
	//	if (demux.IsAudio(pkt)) {
	//		adecoder.Send(pkt);
	//		AVFrame* frame = adecoder.Recv();
	//		cout << "Audio: " << frame << endl;
	//		av_frame_free(&frame);
	//	}
	//	else {
	//		vdecoder.Send(pkt);
	//		AVFrame* frame = vdecoder.Recv();
	//		cout << "Video: " << frame << endl;
	//		av_frame_free(&frame);
	//	}
	//	if (!pkt) break;
	//}
	//TestThread tt;

	QApplication a(argc, argv);
	TinyPlayer w;
	w.show();

	// ³õÊ¼»¯gl´°¿Ú
	//w.ui.openGLWidget->Init(tt.demux.width, tt.demux.height);
	//tt.video = w.ui.openGLWidget;
	//tt.Init();
	//tt.start();
	XDemuxThread dt;
	dt.Open("test.mp4", w.ui.openGLWidget);
	dt.Start();
	return a.exec();
}
