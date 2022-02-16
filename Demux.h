#pragma once
#include<mutex>


class Demux
{
public:
	virtual bool Open(const char*);
	Demux();
	virtual ~Demux();
};

