#include "mainwindow.h"
#include "shader.h"
#include "checkershader.h"
#include "viewwidget.h"
#include "glglobals.h"
#include <QMessageBox>
#include <QGuiApplication>
#include <QCursor>
#include <QImage>

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


