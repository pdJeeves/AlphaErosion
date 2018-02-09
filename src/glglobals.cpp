#include <QOpenGLFunctions_3_2_Core>
#include <vector>
#include "glglobals.h"

static int      g_refCount = 0L;
static uint32_t g_whiteTexture = 0L;
static uint32_t g_vao = 0L;
static uint32_t g_vbo = 0L;

void AddRefGL()
{
	if(++g_refCount > 1)
		return;
}

void ReleaseGL(QOpenGLFunctions_3_2_Core * gl)
{
	if(--g_refCount) return;

	if(g_whiteTexture)
		gl->glDeleteTextures(1, &g_whiteTexture);
	if(g_vao)
		gl->glDeleteVertexArrays(1, &g_vao);
	if(g_vbo)
		gl->glDeleteBuffers(1, &g_vao);

	g_whiteTexture = 0L;
	g_vao = 0L;
	g_vbo = 0L;
}

uint32_t getWhiteTexture(QOpenGLFunctions_3_2_Core * gl)
{
	if(g_whiteTexture)
		return g_whiteTexture;
	else
	{
		gl->glGenTextures(1, &g_whiteTexture);
		gl->glBindTexture(GL_TEXTURE_2D, g_whiteTexture);

		gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}

	std::vector<GLubyte> tex_data;
	tex_data.resize(3*16);

	for(int y = 0; y < 4; ++y)
	{
		for(int x = 0; x < 4; ++x)
		{
			int i = (y * 4 + x)*3;

			tex_data[i  ] = 0xFF;
			tex_data[i+1] = 0xFF;
			tex_data[i+2] = 0xFF;
		}
	}

	gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, &tex_data[0]);
	return g_whiteTexture;
}

void bindVAO(QOpenGLFunctions_3_2_Core * gl)
{
	if(g_vao)
	{
		gl->glBindVertexArray(g_vbo);
		return;
	}

	const GLshort verts[12][5] =
	{
		{-1, -1, 0, 0, 1},
		{ 1, -1, 0, 1, 1},
		{-1,  1, 0, 0, 0},

		{-1,  1, 0, 0, 0},
		{ 1, -1, 0, 1, 1},
		{ 1,  1, 0, 1, 0},

		{-1, -1, 0, 0, 0},
		{ 1, -1, 0, 1, 0},
		{-1,  1, 0, 0, 1},

		{-1,  1, 0, 0, 1},
		{ 1, -1, 0, 1, 0},
		{ 1,  1, 0, 1, 1},
	};

	gl->glGenVertexArrays(1, &g_vao);
	gl->glGenBuffers(1, &g_vbo);

	gl->glBindVertexArray(g_vao);
	gl->glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

	gl->glBufferData(GL_ARRAY_BUFFER, 12 * 5 * sizeof(GLshort), verts, GL_STATIC_DRAW);
	gl->glVertexAttribPointer(0, 3, GL_SHORT, GL_FALSE, sizeof(GLshort)*5, 0L);
	gl->glEnableVertexAttribArray(0);
	gl->glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, sizeof(GLshort)*5, (void*) (sizeof(GLshort)*3));
	gl->glEnableVertexAttribArray(1);
}
