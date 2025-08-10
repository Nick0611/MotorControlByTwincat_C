#pragma once
#include <stdarg.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <string>

using namespace rapidjson;

// check json item existence
inline bool check_json_item(const Value * v, ...)
{
	bool result = true;
	va_list ap;
	va_start(ap, v);
	while (result)
	{
		char * name = va_arg(ap, char*);
		if (strlen(name) == 0)
			break;
		if (!v->HasMember(name))
		{
			result = false;
			fprintf(stderr, "Error: please specify the [%s] field for json config file!\n", name);
		}
	}
	va_end(ap);
	return result;
}

inline bool read_from_file(Document & doc, const char * filename)
{
	FILE * fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open file %s\n", filename);
		fflush(stderr);
		return false;
	}
	char readBuffer[65536] = { 0 };
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	if (doc.ParseStream(is).HasParseError())
	{
		fprintf(stderr, "Parsing file %s failed.\n", filename);
		fflush(stderr);
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

inline bool save_to_file(Document & doc, const char * filename, bool format=true)
{
	StringBuffer buffer;
	if (format)
	{
		PrettyWriter<StringBuffer> w(buffer);
		doc.Accept(w);
	}
	else
	{
		Writer<StringBuffer> w(buffer);
		doc.Accept(w);
	}

	FILE * fp = fopen(filename, "w");
	fprintf(fp, "%s", buffer.GetString());
	fclose(fp);

	return true;
}

inline std::string construct_json_rsp(int id, bool success, const char* err_msg)
{
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	writer.StartObject();
	writer.Key("id");
	writer.Int(id);
	writer.Key("success");
	writer.Bool(success);
	writer.Key("error_msg");
	writer.String(err_msg);
	writer.EndObject();
	return buffer.GetString();
}
