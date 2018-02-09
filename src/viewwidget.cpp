#include "mainwindow.h"
#include "shader.h"
#include "viewwidget.h"
#include "glglobals.h"
#include <QMessageBox>
#include <QGuiApplication>
#include <QImage>

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
}

ViewWidget::~ViewWidget()
{
	if(m_texture[0])
		glDeleteTextures(1, &m_texture[0]);
	if(m_texture[1])
		glDeleteTextures(1, &m_texture[0]);

	ReleaseGL(this);
	g_shader.Release(this);
}

void ViewWidget::setTexture(int id, QImage & image)
{
	if(id == 0)
	{
		m_width = image.width();
		m_height = image.height();
	}

	glEnable(GL_TEXTURE_2D);

	if(m_texture[id] != 0L)
		glBindTexture(GL_TEXTURE_2D, m_texture[id]);
	else
	{
		glGenTextures(1, &m_texture[id]);
		glBindTexture(GL_TEXTURE_2D, m_texture[id]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &tex_data[0]);
	glDisable(GL_TEXTURE_2D);
	repaint();
}

void ViewWidget::clearTexture(int id)
{
	glDeleteTextures(1, &m_texture[id]);
	m_texture[id] = 0L;
}

void 	ViewWidget::initializeGL()
{
	QOpenGLFunctions_3_2_Core::initializeOpenGLFunctions();
}

void ViewWidget::paintGL()
{
	glViewport(0, 0, width(), height());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shader.bindShader(this);
	bindVAO(this);

	glDisable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texture[1]? m_texture[1] : getWhiteTexture(this));

	glUniform1i(g_shader.u_erosion, 0);
	glUniform1i(g_shader.u_gradient, 1);
	glUniform2f(g_shader.u_imageSize, m_width, m_height);
	glUniform2f(g_shader.u_windowSize, width(), height());
	glUniform1f(g_shader.u_fadeInDuration,     w->fadeInDuration());
	glUniform1f(g_shader.u_fadeOutStart,       w->fadeOutStart());
	glUniform1f(g_shader.u_fadeOutDuration,    w->fadeOutDuration());
	glUniform1f(g_shader.u_transitionDuration, w->transitionDuration());
	glUniform1f(g_shader.u_time,               w->progress() * w->durationMs() / 1000.0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glFlush();
}

void 	ViewWidget::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
}



