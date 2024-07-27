#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#define SNPRINTF	_snprintf_s
#define MSLEEP(ms)	Sleep(ms)
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#define SNPRINTF	snprintf
#define MSLEEP(ms)	usleep(ms * 1000)
#endif
#include <math.h>
#include <float.h>

#define		STRNCPY(dst, src)	strncpy(dst, src, sizeof(dst) - 1)
#define		ZEROSTR(str)		memset(str, 0, sizeof(str))

//////////////////////////////////////////////////////////

inline double double_sub(double x, double y)
{
	double ret = x - y;
	return  (fabs(ret) <= FLT_EPSILON) ? 0 : ret;
}

#define		D_SUBEQL(x, y)		x = double_sub(x, y)
#define		D_ADDEQL(x, y)		(x) += (y)
#define		D_EPSGT(x, y)		((x)-(y)) > FLT_EPSILON ? true : false

#define		_MAX(x, y)			(((x) > (y)) ? (x) : (y))
#define		_MIN(x, y)			(((x) < (y)) ? (x) : (y))


inline uint64_t now_us()
{
#ifdef _WIN32
	//  Get the high resolution counter's accuracy.
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceFrequency (&ticksPerSecond);

	//  What time is it?
	LARGE_INTEGER tick;
	QueryPerformanceCounter (&tick);

	//  Convert the tick number into the number of seconds
	//  since the system was started.
	double ticks_div = ticksPerSecond.QuadPart / 1000000.0;
	return (uint64_t) (tick.QuadPart / ticks_div);
#else
	//  Use POSIX clock_gettime function to get precise monotonic time.
    struct timespec tv;
    int rc = clock_gettime (CLOCK_MONOTONIC, &tv);
    // Fix case where system has clock_gettime but CLOCK_MONOTONIC is not supported.
    // This should be a configuration check, but I looked into it and writing an
    // AC_FUNC_CLOCK_MONOTONIC seems beyond my powers.
    if( rc != 0) {
        //  Use POSIX gettimeofday function to get precise time.
        struct timeval tv;
        gettimeofday (&tv, NULL);
        return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
    }
    return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_nsec / 1000);
	//return (tv.tv_sec * (uint64_t) 1000000000 + tv.tv_nsec);
#endif
}


inline void GetCurDateTime ( char * d, size_t dlen, char * t, size_t tlen)
{
	time_t t_t = 0;
	t_t = time (NULL);
	struct tm *gtimeinfo = localtime( &t_t );
	if ( d != NULL )
	{
		strftime(d, dlen, "%Y%m%d", gtimeinfo);
	}
	if ( t != NULL )
	{
		strftime(t, tlen, "%H:%M:%S", gtimeinfo);
	}
}

inline void GetCurDate(int * d)
{
	time_t t_t = 0;
	t_t = time (NULL);
	struct tm *gtimeinfo = localtime( &t_t );
	if ( d != NULL )
	{
		*d = ( gtimeinfo->tm_year + 1900 ) * 10000 + ( gtimeinfo->tm_mon + 1 ) * 100 + gtimeinfo->tm_mday;
	}
}

inline int BindCpu(int i)
{  
#ifdef _WIN32
#else
    cpu_set_t mask;  
    CPU_ZERO(&mask);  
  
    CPU_SET(i,&mask);  
  
    if(-1 == pthread_setaffinity_np(pthread_self() ,sizeof(mask),&mask))  
    {  
        return -1;
    }
#endif
    return 0;  
}

inline int ElevatePriority()
{
	int rc = 0;
#ifndef _WIN32
	struct sched_param    sched;
    int                   policy;
	pthread_t			  id = pthread_self();
	rc = pthread_getschedparam(id, &policy, &sched);
    if (rc != 0) 
	{
        printf("pthread_getschedparam ERROR, ret=%d\n", rc);
    }
    sched.sched_priority = 10; // sched_get_priority_max(SCHED_FIFO ) / 2;
    rc = pthread_setschedparam(id, SCHED_FIFO , &sched);
	if (rc != 0) 
	{
        printf("pthread_setschedparam ERROR, ret=%d\n", rc);
    }
#endif
	return rc;
}

