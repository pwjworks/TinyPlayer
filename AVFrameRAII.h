#pragma once
struct AVFrame;
class AVFrameRAII
{
public:
	AVFrameRAII();
	~AVFrameRAII();
	AVFrameRAII(AVFrame const&) = delete;
	AVFrameRAII& operator=(AVFrame const&) = delete;

	AVFrame* get_frame() { return frame; }
private:
	AVFrame* frame;
};

