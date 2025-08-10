#pragma once
#include "GLSLProgram.h"
#include "glm/glm.hpp"

class GLSLShaderToy : public GLSLProgram
{
public:
    GLSLShaderToy();
    virtual ~GLSLShaderToy();

    bool InitFromShaderFiles(const char* vertex_shader_file, const char* frag_shader_file);
    bool InitFromShaderString(const char* vertex_shader_src, const char* frag_shader_src);

    GLuint GetVAOId() { return m_vao_id; }
    GLuint GetVBOId() { return m_vbo_id; }
    GLuint GetEBOId() { return m_ebo_id; }

    const char* GetShaderToyVertexSrc();
    const char* GetShaderToyFragHeader();
public: // 以下为 Frame update 函数
    // 设置视口
    void SetViewPort(GLint x, GLint y, GLsizei w, GLsizei h);
    // 设置视口背景色
    void SetViewBackground(GLfloat r, GLfloat g, GLfloat b, GLfloat alpha);
    // 设置uniform变量
    void SetOffset(const glm::vec2 & value);
    void SetMousePos(const glm::vec4& value);
    void SetResolution(const glm::vec3& value);
    void SetTime(GLfloat value);
    void SetTexture(GLuint texture_id);
    // 绘制Shader
    void Draw();

    void Reset();
protected:
    void CreateObjects();
    void DeleteObjects();
protected:
    GLuint m_vbo_id = 0;  // Vertex Buffer Object id
    GLuint m_vao_id = 0;  // Vertex Array Object id
    GLuint m_ebo_id = 0;  // Element Buffer Object id
};

