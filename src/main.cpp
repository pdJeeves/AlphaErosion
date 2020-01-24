#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <iostream>
#include <cstdio>

const char * g_vertexShader();
const char * g_fragmentShader();

void CreateShaderFile(const char * filename, const char * shader);

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

//	CreateShaderFile("shader.vert", g_vertexShader());
//	CreateShaderFile("shader.frag", g_fragmentShader());

	QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setVersion(3, 2);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
  QSurfaceFormat::setDefaultFormat(format);

	MainWindow w;
	w.show();

	return a.exec();
}

void CreateShaderFile(const char * filename, const char * shader)
{
	FILE * file = fopen(filename, "r");

	if(!file)
	{
		file = fopen(filename, "w");
		fwrite(shader, 1, strlen(shader), file);
	}

	fclose(file);
}
