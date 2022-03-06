#pragma once
#include <memory>
struct AVFrame;
class AVFrameRAII;
class IVideoCall
{
public:
	virtual void Init(int width, int height) = 0;
	virtual void Repaint(std::shared_ptr<AVFrameRAII> frame) = 0;
};

