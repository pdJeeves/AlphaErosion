#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <iostream>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    std::cerr << "try single buffer" << std::endl;
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
