#include "AVFrameRAII.h"
extern "C" {
#include<libavutil/frame.h>
}

AVFrameRAII::AVFrameRAII() :frame(av_frame_alloc()) {}

AVFrameRAII::~AVFrameRAII() {
	if (frame)
		av_frame_free(&frame);
}