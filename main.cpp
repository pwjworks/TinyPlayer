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


int main(int argc, char* argv[])
{

	QApplication a(argc, argv);
	TinyPlayer w;
	w.show();

	return a.exec();
}
