#pragma once
typedef struct
{
	char	ip[16];
	int		port;
}ST_BusCfg;

typedef struct
{
	char	ip[16];
	int		port;
}ST_LogCfg;

typedef struct
{
	ST_BusCfg bus;
	ST_LogCfg log;
}ST_BusLogCfg;

typedef struct  
{
	double encoder_init_pos[2];
}ST_PPCMCfg;
