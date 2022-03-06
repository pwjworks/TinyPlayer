#include "AVFrameRAII.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

AVFrameRAII::AVFrameRAII() :frame(av_frame_alloc()) {}

AVFrameRAII::~AVFrameRAII() {
	if (frame)
		av_frame_free(&frame);
}