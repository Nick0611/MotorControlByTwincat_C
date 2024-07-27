#pragma once
#include <string>

class RWCom
{
public:
	RWCom();
	~RWCom();
	bool Open(const char * com, int parity, int stopbits, int char_size, int baud_rate);
	void Close();
	std::string ReadLine();
protected:
	struct RWComCtx;
	RWComCtx* m_ctx = nullptr;
};

