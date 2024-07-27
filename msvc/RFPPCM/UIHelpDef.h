#pragma once
#include <vector>
#include <string>


typedef struct
{
	std::string title;
	std::string file;
}ST_UIHelpDocItem;

typedef struct
{
	ST_UIHelpDocItem menu;
	std::vector<ST_UIHelpDocItem> child;
}ST_UIHelpContentItem;

typedef struct
{
	ST_UIHelpDocItem intro;
	std::vector<ST_UIHelpContentItem> contents;
}ST_UIHelpTree;
