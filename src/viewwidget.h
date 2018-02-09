#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H
#include "opengl.h"
#include <QOpenGLWidget>

class MainWindow;

class ViewWidget : public QOpenGLWidget
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
};

#endif // VIEWWIDGET_H
