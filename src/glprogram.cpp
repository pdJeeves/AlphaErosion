#include "glprogram.h"
#include <cstdio>
#include <exception>
#include <vector>
#include <cassert>


glProgram::glProgram() :
	m_refCount(0L),
	m_program(0L)
{
	shaders.fill(0);
}

glProgram::~glProgram()
{
}


void glProgram::AddRef()
{
	lock_guard_t lock(m_mutex);
	++m_refCount;
}

void glProgram::Release(QOpenGLFunctions_3_2_Core* gl)
{
	lock_guard_t lock(m_mutex);
	if( --m_refCount == 0 )
	{
		destruct(gl);
	}

	assert(m_refCount >= 0);
}

void glProgram::destruct(QOpenGLFunctions_3_2_Core * gl)
{
	if(m_program)
	{
		for(size_t i = 0; i < shaders.size(); ++i)
		{
			gl->glDeleteShader(shaders[i]);
			shaders[i] = 0L;
		}

		gl->glDeleteProgram(m_program);
		m_program = 0L;
	}
}

void glProgram::printLog(QOpenGLFunctions_3_2_Core * gl)
{
	GLint length;
	gl->glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);

	std::vector<char> log(length, 0);
	gl->glGetProgramInfoLog(m_program, length, &length, log.data());
	fprintf(stderr, "%s", log.data());
	fflush(stderr);
}

int glProgram::getShaderIndex(GLuint type)
{
	switch(type)
	{
	default: throw std::logic_error("unknown shader type");
	case GL_COMPUTE_SHADER:			return 0;
	case GL_VERTEX_SHADER:			return 1;
	case GL_TESS_CONTROL_SHADER:	return 2;
	case GL_TESS_EVALUATION_SHADER:	return 3;
	case GL_GEOMETRY_SHADER:		return 4;
	case GL_FRAGMENT_SHADER:		return 5;
	}
}

void glProgram::compile(QOpenGLFunctions_3_2_Core * gl, const char * text, GLuint type)
{
	int i = getShaderIndex(type);

	if(shaders[i])
	{
		throw std::logic_error("tried to add shader type which already exists");
	}

	shaders[i] = compileShader(gl, text, type);

	if(shaders[i] == 0)
	{
		throw std::logic_error("failed to add shader");
	}

	gl->glAttachShader(m_program, shaders[i]);
}


void glProgram::load(QOpenGLFunctions_3_2_Core * gl, const char * filename, GLuint type)
{
	int i = getShaderIndex(type);

	if(shaders[i])
	{
		throw std::logic_error("tried to add shader type which already exists");
	}

	shaders[i] = loadShader(gl, filename, type);

	if(shaders[i] == 0)
	{
		throw std::logic_error("failed to add shader");
	}

	gl->glAttachShader(m_program, shaders[i]);
}

void glProgram::link(QOpenGLFunctions_3_2_Core * gl)
{
	gl->glLinkProgram(m_program);

	GLint status;
	gl->glGetProgramiv(m_program, GL_LINK_STATUS, &status);

	if(status == GL_FALSE)
	{
		printLog(gl);
		throw std::runtime_error("link failed");
	}
}

bool glProgram::validate(QOpenGLFunctions_3_2_Core * gl)
{
	gl->glValidateProgram(m_program);

	GLint status;
	gl->glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status);

	if(status == GL_FALSE)
	{
		printLog(gl);
	}

	return status == GL_TRUE;
}

void glProgram::attribute(QOpenGLFunctions_3_2_Core * gl, GLuint index, const char * name)
{
	gl->glBindAttribLocation(m_program, index, name);
}

void glProgram::uniform(QOpenGLFunctions_3_2_Core * gl, GLint & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

bool glProgram::bindShader(QOpenGLFunctions_3_2_Core * gl)
{
static glProgram * current_program = 0L;
	if(current_program == this)
		return false;
	current_program = this;

	if(!m_program)
	{
		m_program = gl->glCreateProgram();
		construct(gl);
	}

	gl->glUseProgram(m_program);
	return true;
}

GLuint glProgram::compileShader(QOpenGLFunctions_3_2_Core * gl, const char * text, GLuint shader_type)
{
	GLuint shader = 0;
	GLint length = 0;
	std::vector<char> error;

	shader = gl->glCreateShader(shader_type);

	if(!shader)
	{
		fprintf(stderr, "unable to allocate glsl shader");
		return 0;
	}

	gl->glShaderSource(shader, 1, &text, 0L);
	gl->glCompileShader(shader);

	GLint success = 0;
	gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(success != GL_FALSE)
	{
		return shader;
	}

	length = 0;
	gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

	error.resize(length, 0);
	gl->glGetShaderInfoLog(shader, length, &length, error.data());

	fprintf(stderr, "%s", error.data());
	gl->glDeleteShader(shader);
	return 0;
}

GLuint glProgram::loadShader(QOpenGLFunctions_3_2_Core * gl, const char * filename, GLuint shader_type)
{
	GLuint shader;
	GLint length;
	std::vector<char> text;
	char * text_ptr;

	FILE * file = fopen(filename, "r");

	if(!file)
	{
		fprintf(stderr, "unable to open glsl shader '%s'", filename);
		return 0;
	}

	shader = gl->glCreateShader(shader_type);

	if(!shader)
	{
		fclose(file);
		fprintf(stderr, "unable to allocate glsl shader");
		return 0;
	}

	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	text.resize(length, 0);
	fread(text.data(), 1, text.size(), file);
	fclose(file);

	text_ptr = text.data();
	gl->glShaderSource(shader, 1, &text_ptr, &length);
	gl->glCompileShader(shader);

	GLint success = 0;
	gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(success != GL_FALSE)
	{
		return shader;
	}

	length = 0;
	gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	text.resize(length, 0);
	gl->glGetShaderInfoLog(shader, length, &length, text.data());

	fprintf(stderr, "%s", text.data());
	gl->glDeleteShader(shader);
	return 0;
}
