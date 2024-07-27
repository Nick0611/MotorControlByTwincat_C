#include "UICfgParser.h"
#include <fstream>
#include "UIHelpDefSerializeTool.h"
#include "TimeCounter.h"
#include "Config.h"


#define CFG_FILE "uicfg.json"
#define HELP_TREE_FILE "help/help.json"

UICfgParser* UICfgParser::Inst()
{
	static UICfgParser p;
	return &p;
}


bool UICfgParser::LoadCfg()
{
	TimeCounter t;
	// parse json configure file
	{
		std::ifstream isf(CFG_FILE);
		if (!isf.is_open())
		{
			fprintf(stderr, "File %s does not exists!\n", CFG_FILE);
			return false;
		}
		try
		{
			cereal::JSONInputArchive ar(isf);
			ar(cereal::make_nvp("bus", m_cpscfg.bus));
			ar(cereal::make_nvp("log", m_cpscfg.log));
			ar(cereal::make_nvp("app_id", m_app_id));
			ar(cereal::make_nvp("opt_server_id", m_opt_server_id));
			ar(cereal::make_nvp("sensor_server_id", m_sensor_server_id));
			ar(cereal::make_nvp("hypersen_server_id", m_hypersen_server_id));
			ar(cereal::make_nvp("finray_calc_server_id", m_finray_calc_server_id));
			ar(cereal::make_nvp("ppcm", m_ppcm_cfg));
		}
		catch (const std::exception& e)
		{
			fprintf(stderr, "Parsing %s exception: %s\n", CFG_FILE, e.what());
			return false;
		}
	}
	// parse help file
#ifdef UI_CFG_BUILD_DOC
	{
		std::ifstream isf(HELP_TREE_FILE);
		if (!isf.is_open())
		{
			fprintf(stderr, "File %s does not exists!\n", HELP_TREE_FILE);
		}
		else
		{
			try
			{
				cereal::JSONInputArchive ar(isf);
				ar(cereal::make_nvp("intro", m_help_tree.intro));
				ar(cereal::make_nvp("contents", m_help_tree.contents));
			}
			catch (const std::exception& e)
			{
				fprintf(stderr, "Parsing %s exception: %s\n", HELP_TREE_FILE, e.what());
			}
		}
	}
#endif
	t.Tick(__FUNCTION__);
	return true;
}
