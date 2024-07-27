#pragma once
#include "UICfg.h"
#include "UIHelpDef.h"

class UICfgParser
{
public:
	static UICfgParser* Inst();

	bool LoadCfg();

public:
	// bus log cfg
	ST_BusLogCfg m_cpscfg = { 0 };
	// ui app id
	int m_app_id = -1;

	int m_opt_server_id = -1;
	int m_sensor_server_id = -1;
	int m_hypersen_server_id = -1;
	int m_finray_calc_server_id = -1;
	// help tree
	ST_UIHelpTree m_help_tree;
	// ppcm cfg
	ST_PPCMCfg m_ppcm_cfg = { 0 };
private:
	UICfgParser() = default;
};

// 全局静态变量
static UICfgParser * g_cfg = UICfgParser::Inst();
