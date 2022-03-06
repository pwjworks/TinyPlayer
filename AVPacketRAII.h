#pragma once
struct AVPacket;
class AVPacketRAII
{
public:
	AVPacketRAII();
	~AVPacketRAII();
	AVPacketRAII(AVPacket const&) = delete;
	AVPacketRAII& operator=(AVPacket const&) = delete;

	AVPacket* get_avpacket() { return avpacket; }
private:
	AVPacket* avpacket;
};

