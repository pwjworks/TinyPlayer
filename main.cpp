#include "tinyplayer.h"
#include "XDemux.h"
#include "XResample.h"
#include "XDecode.h"
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

		//demux.Clear();
		//demux.Close();
		cout << "demux.Open = " << demux.Open("test.mp4");
		//cout << "CopyVPara: " << demux.CopyVPara() << endl;
		//cout << "CopyAPara: " << demux.CopyAPara() << endl;
		//cout << "seed = " << demux.Seek(0.5) << endl;

		cout << "vdecoder.Open =" << vdecoder.Open(demux.CopyVPara()) << endl;
		cout << "adecoder.Open =" << adecoder.Open(demux.CopyAPara()) << endl;
		resample.Open(demux.CopyAPara());
	}
	unsigned char* pcm = new unsigned char[1024 * 1024];
	void run() {
		for (;;) {
			AVPacket* pkt = demux.Read();
			if (demux.IsAudio(pkt)) {
				adecoder.Send(pkt);
				AVFrame* frame = adecoder.Recv();
				resample.Resample(frame, pcm);

				cout << "Resample:" << resample.Resample(frame, pcm) << " ";
				//cout << "Audio: " << frame << endl;
				//av_frame_free(&frame);
			}
			else {
				vdecoder.Send(pkt);
				AVFrame* frame = vdecoder.Recv();
				cout << "Video: " << frame << endl;
				video->Repaint(frame);
				//av_frame_free(&frame);
			}
			if (!pkt) break;
		}
	}
	XDemux demux;
	XDecode vdecoder;
	XDecode adecoder;
	XResample resample;
	XVideoWidget* video;
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
	TestThread tt;
	tt.Init();
	QApplication a(argc, argv);
	TinyPlayer w;
	w.show();

	// ��ʼ��gl����
	w.ui.openGLWidget->Init(tt.demux.width, tt.demux.height);
	tt.video = w.ui.openGLWidget;
	tt.start();
	return a.exec();
}
