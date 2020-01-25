#include "mainwindow.h"
#include "shader.h"
#include "checkershader.h"
#include "viewwidget.h"
#include "glglobals.h"
#include "gif.h"
#include <QMessageBox>
#include <QGuiApplication>
#include <QCursor>
#include <QImage>
#include <iostream>

#define GL_ASSERT glAssert();

ViewWidget::ViewWidget(QWidget * m_parent) :
QOpenGLWidget(m_parent),
	w(0L),
	m_width(0L),
	m_height(0L)
{
	m_texture[0] = 0L;
	m_texture[1] = 0L;
	AddRefGL();
	g_shader.AddRef();
    g_checker.AddRef();
}

ViewWidget::~ViewWidget()
{
	if(m_texture[0])
    {
        glDeleteTextures(1, &m_texture[0]); GL_ASSERT
    }
	if(m_texture[1])
    {
        glDeleteTextures(1, &m_texture[0]); GL_ASSERT
    }

	ReleaseGL(this);
	g_shader.Release(this);
    g_checker.Release(this);
}

void ViewWidget::setTexture(int id, QImage & image)
{
	if(id == 0)
	{
		m_width = image.width();
		m_height = image.height();
	}

	if(m_texture[id] != 0L)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture[id]); GL_ASSERT
    }
	else
	{
        glGenTextures(1, &m_texture[id]); GL_ASSERT
        glBindTexture(GL_TEXTURE_2D, m_texture[id]); GL_ASSERT
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GL_ASSERT
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GL_ASSERT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GL_ASSERT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GL_ASSERT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); GL_ASSERT
	}

	std::vector<GLubyte> tex_data;
    tex_data.resize(image.width() * image.height() * 4);

	for(int y = 0; y < image.height(); ++y)
	{
		for(int x = 0; x < image.width(); ++x)
		{
			int i = y * image.width() + x;
			QRgb px = image.pixel(x, y);

			tex_data[i*4+0] = qRed(px);
			tex_data[i*4+1] = qGreen(px);
			tex_data[i*4+2] = qBlue(px);
			tex_data[i*4+3] = qAlpha(px);
		}
	}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &tex_data[0]); GL_ASSERT
	repaint();
}

void ViewWidget::clearTexture(int id)
{
	glDeleteTextures(1, &m_texture[id]);
	m_texture[id] = 0L;

	if(id == 0)
	{
		m_width = 0;
		m_height = 0;
	}
}

void 	ViewWidget::initializeGL()
{
    QOpenGLFunctions_3_2_Core::initializeOpenGLFunctions(); GL_ASSERT
    glClearColor(1, 0, 0, 1); GL_ASSERT
    glDisable(GL_DEPTH_TEST); GL_ASSERT
    glEnable(GL_BLEND); GL_ASSERT
    glBlendEquation(GL_FUNC_ADD); GL_ASSERT
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GL_ASSERT
}

void ViewWidget::paintGL()
{
	QPoint mouse = mapFromGlobal(QCursor::pos());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_ASSERT

    bindVAO(this);
    g_checker.bindShader(this);
    glUniform2f(g_checker.u_windowSize, width(), height()); GL_ASSERT
    glDrawArrays(GL_TRIANGLES, 0, 6); GL_ASSERT

    g_shader.bindShader(this);
    glActiveTexture(GL_TEXTURE0); GL_ASSERT
    glBindTexture(GL_TEXTURE_2D, m_texture[0]); GL_ASSERT
    glActiveTexture(GL_TEXTURE1); GL_ASSERT
    glBindTexture(GL_TEXTURE_2D, m_texture[1]? m_texture[1] : getWhiteTexture(this)); GL_ASSERT

    glUniform1i(g_shader.u_erosion, 0); GL_ASSERT
    glUniform1i(g_shader.u_gradient, 1); GL_ASSERT
    glUniform2f(g_shader.u_imageSize, m_width, m_height); GL_ASSERT
    glUniform2f(g_shader.u_windowSize, width(), height()); GL_ASSERT
	glUniform2f(g_shader.u_mousePos,   mouse.x(), mouse.y()); GL_ASSERT
    glUniform1f(g_shader.u_fadeInDuration,     w->fadeInDuration()); GL_ASSERT
    glUniform1f(g_shader.u_fadeOutStart,       w->fadeOutStart()); GL_ASSERT
    glUniform1f(g_shader.u_fadeOutDuration,    w->fadeOutDuration()); GL_ASSERT
    glUniform1f(g_shader.u_transitionDuration, w->transitionDuration()); GL_ASSERT
	glUniform1f(g_shader.u_timeOfDayMs,        w->timeOfDayMs()); GL_ASSERT
    glUniform1f(g_shader.u_time,               w->progress() * w->durationMs() / 1000.0); GL_ASSERT

    glDrawArrays(GL_TRIANGLES, 0, 6); GL_ASSERT
}

void 	ViewWidget::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
    glViewport(0, 0, w, h);
}


#include <GL/glu.h>

void ViewWidget::displayOpenGlError(const char * file, const char * function, int line)
{
    GLenum error = glGetError();

    if(error == GL_NO_ERROR) return;

    do
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("FILE: %1\nFUNC: %2\nLINE: %3\nERROR: %4")
                                 .arg(file).arg(function).arg(line).arg((const char *) gluErrorString(error)));

    } while((error = glGetError()) != GL_NO_ERROR);

    w->close();
}


const GLenum g_DrawBuffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

void ViewWidget::ExportGif(std::string const& filename, float fade_in, float fade_out_start, float fade_out, float transition, int frame_delay, int height)
{
	int width = -1;

	if(height == -1)
	{
		height = m_height;
		width  = m_width;
	}
	else
	{
		width = m_width * height / m_height;
	}

	makeCurrent();

	uint32_t fbo[2];
	uint32_t buffers[2];
	uint32_t stencil;

	glGenTextures(2, buffers); GL_ASSERT;
	glGenRenderbuffers(1, &stencil); GL_ASSERT;
	glGenFramebuffers(2, fbo); GL_ASSERT;

	glBindRenderbuffer(GL_RENDERBUFFER, stencil); GL_ASSERT;
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height); GL_ASSERT;

	for(int i = 0; i < 2; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, buffers[i]); GL_ASSERT;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GL_ASSERT;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GL_ASSERT;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GL_ASSERT;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GL_ASSERT;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); GL_ASSERT;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); GL_ASSERT;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]); GL_ASSERT;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffers[i], 0, 0); GL_ASSERT;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stencil); GL_ASSERT;
		glDrawBuffers(1, g_DrawBuffers); GL_ASSERT;

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
	}

//bind VAO
	bindVAO(this);
	glViewport(0, 0, width, height);

//bind uniforms
	g_shader.bindShader(this);
    glActiveTexture(GL_TEXTURE0); GL_ASSERT
    glBindTexture(GL_TEXTURE_2D, m_texture[0]); GL_ASSERT
    glActiveTexture(GL_TEXTURE1); GL_ASSERT
    glBindTexture(GL_TEXTURE_2D, m_texture[1]? m_texture[1] : getWhiteTexture(this)); GL_ASSERT

    glUniform1i(g_shader.u_erosion, 0); GL_ASSERT
    glUniform1i(g_shader.u_gradient, 1); GL_ASSERT
    glUniform2f(g_shader.u_imageSize, 1, 1); GL_ASSERT
    glUniform2f(g_shader.u_windowSize, 1, 1); GL_ASSERT
	glUniform2f(g_shader.u_mousePos,   .5f, .5f); GL_ASSERT
    glUniform1f(g_shader.u_fadeInDuration,     fade_in); GL_ASSERT
    glUniform1f(g_shader.u_fadeOutStart,       fade_out_start); GL_ASSERT
    glUniform1f(g_shader.u_fadeOutDuration,    fade_out); GL_ASSERT
    glUniform1f(g_shader.u_transitionDuration, transition); GL_ASSERT
	glUniform1f(g_shader.u_timeOfDayMs,        w->timeOfDayMs()); GL_ASSERT

	float duration = fade_in + std::max(0.f, fade_out - fade_out_start);
	int total_frames = duration * 100 / frame_delay + 2;

	std::unique_ptr<uint8_t[]> frame(new uint8_t[width*height*4]);

	GifWriter g;
	GifBegin(&g, filename.c_str(), width, height, frame_delay);

	glActiveTexture(GL_TEXTURE3);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	for(int i = 0; i < total_frames; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i&1]);  GL_ASSERT
		glFinish();  GL_ASSERT

//write current frame
		if(i >= 2)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0); GL_ASSERT
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); GL_ASSERT
			glReadPixels(0, 0, width, height, GL_RGBA,GL_UNSIGNED_INT_8_8_8_8, &frame[0]); GL_ASSERT

			GifWriteFrame(&g, &frame[0], width, height, frame_delay); GL_ASSERT
		}

		glClear(GL_COLOR_BUFFER_BIT);

//set time
		glUniform1f(g_shader.u_time,  i * frame_delay / 100.0); GL_ASSERT

		glDrawArrays(GL_TRIANGLES, 0, 6); GL_ASSERT
	}

	GifEnd(&g);


    glUniform1f(g_shader.u_time,               w->progress() * w->durationMs() / 1000.0); GL_ASSERT

//clean up
	glDeleteFramebuffers(2, fbo);
	glDeleteRenderbuffers(1, &stencil);
	glDeleteTextures(2, buffers);

	glViewport(0, 0, this->width(), this->height());

	doneCurrent();
}
