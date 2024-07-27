#pragma once
#include <fstream>
#include "CPSCfgSerializeTool.h"

#define OPT_CFG_FILE	"optcfg.json"

class OptCfgParser
{
public:
	static OptCfgParser* Inst()
	{
		static OptCfgParser cfg;
		return &cfg;
	}

	bool LoadCfg()
	{
		// parse json configure file
		{
			std::ifstream isf(OPT_CFG_FILE);
			if (!isf.is_open())
			{
				fprintf(stderr, "File %s does not exists!\n", OPT_CFG_FILE);
				return false;
			}
			try
			{
				cereal::JSONInputArchive ar(isf);
				ar(cereal::make_nvp("bus", m_cpscfg.bus));
				ar(cereal::make_nvp("log", m_cpscfg.log));
				ar(cereal::make_nvp("dev_id", m_dev_id));
				ar(cereal::make_nvp("push_freq_hz", m_push_freq_hz));
				ar(cereal::make_nvp("opt", m_opt_cfg));
			}
			catch (const std::exception& e)
			{
				fprintf(stderr, "Parsing %s exception: %s\n", OPT_CFG_FILE, e.what());
				return false;
			}
		}
		return true;
	}

public:
	// bus log cfg
	ST_BusLogCfg m_cpscfg = { 0 };
	// dev id
	int m_dev_id = -1;
	int m_push_freq_hz = 10;
	// opt cfg
	ST_OptServerCfg m_opt_cfg = { 0 };
private:
	OptCfgParser() = default;
};

// 全局静态变量
static OptCfgParser* g_cfg = OptCfgParser::Inst();
