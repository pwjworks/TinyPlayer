#include "AVPacketRAII.h"

extern "C" {
#include "libavformat/avformat.h"
}

AVPacketRAII::AVPacketRAII() :avpacket(av_packet_alloc()) {};

AVPacketRAII::~AVPacketRAII() {
	if (avpacket)
		av_free_packet(avpacket);
}