#pragma once
#include "glad/glad.h"


class GLSLProgram
{
public:
	enum SHADER_TYPE 
	{ 
		VERTEX = 0, 
		FRAGMENT, 
		GEOMETRY, 
		TESSELATION,
		SHADER_TYPE_COUNT
	};

	GLSLProgram();
	virtual ~GLSLProgram();

	bool LoadShaderFromFile(const char* filename, SHADER_TYPE type);
	bool LoadShaderFromString(const char* shader_str, SHADER_TYPE type);

	bool CreateProgram();
	void DeleteProgram();

	void UseProgram();

	GLuint GetProgramID();
protected:
	bool CheckCompileStatus(SHADER_TYPE type);
	bool CheckLinkStatus();
protected:
	GLuint m_program_id = 0;
	GLuint m_shader_ids[SHADER_TYPE_COUNT] = { 0 };
};

