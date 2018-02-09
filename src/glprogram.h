#ifndef GLPROGRAM_H
#define GLPROGRAM_H
#include "opengl.h"
#include <memory>
#include <array>
#include <cstdint>
#include <vector>
#include <mutex>


class glProgram
{
typedef std::mutex mutex_t;
typedef std::lock_guard<mutex_t> lock_guard_t;
private:
	std::mutex m_mutex;
	int m_refCount;
	GLuint m_program;

	std::array<GLuint,6>  shaders;

	static GLuint compileShader(const char * filename, GLuint);
	static GLuint loadShader(const char * filename, GLuint);
	static int getShaderIndex(GLuint);
	void printLog();

protected:
	void create();
	void compile(const char * text, GLuint);
	void load(const char * filename, GLuint);
	void attribute(GLuint index, const char * name);
	void link();
	void uniform(GLint & uniform, const char * name);

public:
	bool isGood() const { return m_program; }

	glProgram();
	virtual ~glProgram();

	glProgram(const glProgram & it) = delete;
	glProgram(glProgram && it) = delete;
	glProgram & operator=(const glProgram & it) = delete;
	glProgram & operator=(glProgram &&) = delete;

	virtual void construct() = 0;
	void destruct();

	void AddRef();
	void Release();

	bool validate();

	bool bindShader();
};

#endif // GLSHADER_H
