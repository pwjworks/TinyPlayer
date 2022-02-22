#pragma once

struct SwrContext;
class SwrContext_RAII
{
public:
	SwrContext_RAII();
	~SwrContext_RAII();
private:
	SwrContext* actx;
};

