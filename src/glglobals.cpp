#include "glglobals.h"
#include <vector>

static int    g_refCount = 0L;
static GLuint g_whiteTexture = 0L;
static GLuint g_vao = 0L;
static GLuint g_vbo = 0L;

void AddRefGL()
{
	if(++g_refCount > 1)
		return;
}

void ReleaseGL()
{
	if(--g_refCount) return;

	if(g_whiteTexture)
		glDeleteTextures(1, &g_whiteTexture);
	if(g_vao)
		glDeleteVertexArrays(1, &g_vao);
	if(g_vbo)
		glDeleteBuffers(1, &g_vao);

	g_whiteTexture = 0L;
	g_vao = 0L;
	g_vbo = 0L;
}

GLuint getWhiteTexture()
{
	if(g_whiteTexture)
		return g_whiteTexture;
	else
	{
		glGenTextures(1, &g_whiteTexture);
		glBindTexture(GL_TEXTURE_2D, g_whiteTexture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, &tex_data[0]);
	return g_whiteTexture;
}

void bindVAO()
{
	if(g_vao)
	{
		glBindVertexArray(g_vbo);
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

	glGenVertexArrays(1, &g_vao);
	glGenBuffers(1, &g_vbo);

	glBindVertexArray(g_vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);

	glBufferData(GL_ARRAY_BUFFER, 12 * 5 * sizeof(GLshort), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_SHORT, GL_FALSE, sizeof(GLshort)*5, 0L);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, sizeof(GLshort)*5, (void*) (sizeof(GLshort)*3));
	glEnableVertexAttribArray(1);
}
