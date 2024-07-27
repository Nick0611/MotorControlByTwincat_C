#pragma once
#include <time.h>

inline void GetTimeStr(char* dt, size_t len)
{
	time_t t = time(NULL);
	struct tm* gtimeinfo = localtime(&t);
	strftime(dt, len, "%Y%m%d%H%M%S", gtimeinfo);
}
