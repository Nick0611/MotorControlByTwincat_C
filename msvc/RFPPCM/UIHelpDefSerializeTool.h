#pragma once
#include "UICfgSerializeTool.h"

template<class Archive>
void serialize(Archive& archive, ST_UIHelpDocItem& m)
{
	archive(N(m, title), N(m, file));
}

template<class Archive>
void serialize(Archive& archive, ST_UIHelpContentItem& m)
{
	archive(N(m, menu), N(m, child));
}

template<class Archive>
void serialize(Archive& archive, ST_UIHelpTree& m)
{
	archive(N(m, intro), N(m, contents));
}
