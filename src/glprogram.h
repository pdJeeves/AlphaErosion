#ifndef GLPROGRAM_H
#define GLPROGRAM_H
#include "viewwidget.h"
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

    static GLuint compileShader(ViewWidget * gl, const char * filename, GLuint);
    static GLuint loadShader(ViewWidget * gl, const char * filename, GLuint);
	static int getShaderIndex(GLuint);
    void printLog(ViewWidget * gl);

protected:
	void create();
    void compile(ViewWidget * gl, const char * text, GLuint);
    void load(ViewWidget * gl, const char * filename, GLuint);
    void attribute(ViewWidget * gl, GLuint index, const char * name);
    void link(ViewWidget * gl);
    void uniform(ViewWidget * gl, GLint & uniform, const char * name);

public:
	bool isGood() const { return m_program; }

	glProgram();
	virtual ~glProgram();

	glProgram(const glProgram & it) = delete;
	glProgram(glProgram && it) = delete;
	glProgram & operator=(const glProgram & it) = delete;
	glProgram & operator=(glProgram &&) = delete;

    virtual void construct(ViewWidget * gl) = 0;
    void destruct(ViewWidget * gl);

	void AddRef();
    void Release(ViewWidget*);

    bool validate(ViewWidget * gl);

    bool bindShader(ViewWidget * gl);
};

#endif // GLSHADER_H
