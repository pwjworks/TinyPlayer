#include "tinyplayer.h"
#include"XDemux.h"
#include"XDecode.h"
#include <QtWidgets/QApplication>
#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace std;

int main(int argc, char* argv[])
{
	XDemux demux;
	cout << "demux.open = " << demux.Open("test.mp4");

	demux.Clear();
	demux.Close();
	cout << "demux.Open = " << demux.Open("test.mp4");
	cout << "CopyVPara: " << demux.CopyVPara() << endl;
	cout << "CopyAPara: " << demux.CopyAPara() << endl;
	cout << "seed = " << demux.Seek(0.5) << endl;

	XDecode vdecoder;
	cout << "vdecoder.Open =" << vdecoder.Open(demux.CopyVPara()) << endl;
	//vdecoder.Clear();
	//vdecoder.Close();
	XDecode adecoder;
	cout << "adecoder.Open =" << adecoder.Open(demux.CopyAPara()) << endl;
	for (;;) {
		AVPacket* pkt = demux.Read();
		if (demux.IsAudio(pkt)) {
			adecoder.Send(pkt);
			AVFrame* frame = adecoder.Recv();
			cout << "Audio: " << frame << endl;
			av_frame_free(&frame);
		}
		else {
			vdecoder.Send(pkt);
			AVFrame* frame = vdecoder.Recv();
			cout << "Video: " << frame << endl;
			av_frame_free(&frame);
		}
		if (!pkt) break;
	}

	QApplication a(argc, argv);
	TinyPlayer w;
	w.show();
	return a.exec();
}
