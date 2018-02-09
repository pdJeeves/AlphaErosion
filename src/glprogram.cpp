#include "glprogram.h"
#include "opengl.h"
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
	destruct();
}


void glProgram::AddRef()
{
	lock_guard_t lock(m_mutex);
	++m_refCount;
}

void glProgram::Release()
{
	lock_guard_t lock(m_mutex);
	if( --m_refCount == 0 )
	{
		destruct();
	}

	assert(m_refCount >= 0);
}

void glProgram::destruct()
{
	if(m_program)
	{
		for(size_t i = 0; i < shaders.size(); ++i)
		{
			glDeleteShader(shaders[i]);
			shaders[i] = 0L;
		}

		glDeleteProgram(m_program);
		m_program = 0L;
	}
}

void glProgram::printLog()
{
	GLint length;
	glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);

	std::vector<char> log(length, 0);
	glGetProgramInfoLog(m_program, length, &length, log.data());
	fprintf(stderr, "%s", log.data());
	fflush(stderr);
}

int glProgram::getShaderIndex(uint32_t type)
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

void glProgram::compile(const char * text, GLuint type)
{
	int i = getShaderIndex(type);

	if(shaders[i])
	{
		throw std::logic_error("tried to add shader type which already exists");
	}

	shaders[i] = compileShader(text, type);

	if(shaders[i] == 0)
	{
		throw std::logic_error("failed to add shader");
	}

	glAttachShader(m_program, shaders[i]);
}


void glProgram::load(const char * filename, GLuint type)
{
	int i = getShaderIndex(type);

	if(shaders[i])
	{
		throw std::logic_error("tried to add shader type which already exists");
	}

	shaders[i] = loadShader(filename, type);

	if(shaders[i] == 0)
	{
		throw std::logic_error("failed to add shader");
	}

	glAttachShader(m_program, shaders[i]);
}

void glProgram::link()
{
	glLinkProgram(m_program);

	GLint status;
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);

	if(status == GL_FALSE)
	{
		printLog();
		throw std::runtime_error("link failed");
	}
}

bool glProgram::validate()
{
	glValidateProgram(m_program);

	GLint status;
	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status);

	if(status == GL_FALSE)
	{
		printLog();
	}

	return status == GL_TRUE;
}

void glProgram::attribute(GLuint index, const char * name)
{
	glBindAttribLocation(m_program, index, name);
}

void glProgram::uniform(GLint & index, const char * name)
{
	index = glGetUniformLocation(m_program, name);
}

bool glProgram::bindShader()
{
static glProgram * current_program = 0L;
	if(current_program == this)
		return false;
	current_program = this;

	if(!m_program)
	{
		m_program = glCreateProgram();
		construct();
	}

	glUseProgram(m_program);
	return true;
}

GLuint glProgram::compileShader(const char * text, GLuint shader_type)
{
	GLuint shader = 0;
	GLint length = 0;
	std::vector<char> error;

	shader = glCreateShader(shader_type);

	if(!shader)
	{
		fprintf(stderr, "unable to allocate glsl shader");
		return 0;
	}

	glShaderSource(shader, 1, &text, 0L);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(success != GL_FALSE)
	{
		return shader;
	}

	length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

	error.resize(length, 0);
	glGetShaderInfoLog(shader, length, &length, error.data());

	fprintf(stderr, "%s", error.data());
	glDeleteShader(shader);
	return 0;
}

GLuint glProgram::loadShader(const char * filename, GLuint shader_type)
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

	shader = glCreateShader(shader_type);

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
	glShaderSource(shader, 1, &text_ptr, &length);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(success != GL_FALSE)
	{
		return shader;
	}

	length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	text.resize(length, 0);
	glGetShaderInfoLog(shader, length, &length, text.data());

	fprintf(stderr, "%s", text.data());
	glDeleteShader(shader);
	return 0;
}
