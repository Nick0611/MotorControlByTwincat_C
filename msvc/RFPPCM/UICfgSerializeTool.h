#pragma once
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/map.hpp>
//#include <cereal/types/variant.hpp>
#include <cereal/archives/json.hpp>
#include "jsonutil.h"
#include "UICfg.h"


#define N(O, M)				::cereal::make_nvp(#M, O.M)
#define MN(name, value)		::cereal::make_nvp(name, value)


template<class Archive>
void serialize(Archive& archive, ST_BusCfg& m)
{
	archive(N(m, ip), N(m, port));
}

template<class Archive>
void serialize(Archive& archive, ST_LogCfg& m)
{
	archive(N(m, ip), N(m, port));
}

template<class Archive>
void serialize(Archive& archive, ST_BusLogCfg& m)
{
	archive(N(m, bus), N(m, log));
}

template<class Archive>
void serialize(Archive& archive, ST_PPCMCfg& m)
{
	archive(N(m, encoder_init_pos));
}
