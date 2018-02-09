#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLWidget>

class MainWindow;

class ViewWidget : public QOpenGLWidget, public QOpenGLFunctions_3_2_Core
{
friend class MainWindow;
	MainWindow * w;

	GLuint m_texture[2];
	int m_width;
	int m_height;

public:
	ViewWidget(QWidget * m_parent);
	~ViewWidget();

	void setTexture(int id, QImage & image);
	void clearTexture(int id);

	void 	initializeGL();
	void 	paintGL();
	void 	resizeGL(int w, int h);

    void    displayOpenGlError(const char * file, const char * function, int line);
};

#define glAssert() displayOpenGlError(__FILE__, __FUNCTION__, __LINE__);

#endif // VIEWWIDGET_H
