#ifndef GLPROGRAM_H
#define GLPROGRAM_H
#include <QOpenGLFunctions_3_2_Core>
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

	static GLuint compileShader(QOpenGLFunctions_3_2_Core * gl, const char * filename, GLuint);
	static GLuint loadShader(QOpenGLFunctions_3_2_Core * gl, const char * filename, GLuint);
	static int getShaderIndex(GLuint);
	void printLog(QOpenGLFunctions_3_2_Core * gl);

protected:
	void create();
	void compile(QOpenGLFunctions_3_2_Core * gl, const char * text, GLuint);
	void load(QOpenGLFunctions_3_2_Core * gl, const char * filename, GLuint);
	void attribute(QOpenGLFunctions_3_2_Core * gl, GLuint index, const char * name);
	void link(QOpenGLFunctions_3_2_Core * gl);
	void uniform(QOpenGLFunctions_3_2_Core * gl, GLint & uniform, const char * name);

public:
	bool isGood() const { return m_program; }

	glProgram();
	virtual ~glProgram();

	glProgram(const glProgram & it) = delete;
	glProgram(glProgram && it) = delete;
	glProgram & operator=(const glProgram & it) = delete;
	glProgram & operator=(glProgram &&) = delete;

	virtual void construct(QOpenGLFunctions_3_2_Core * gl) = 0;
	void destruct(QOpenGLFunctions_3_2_Core * gl);

	void AddRef();
	void Release(QOpenGLFunctions_3_2_Core*);

	bool validate(QOpenGLFunctions_3_2_Core * gl);

	bool bindShader(QOpenGLFunctions_3_2_Core * gl);
};

#endif // GLSHADER_H
