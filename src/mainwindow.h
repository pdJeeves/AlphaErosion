#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QFileInfo>
#include <chrono>

class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	double fadeInDuration() const;
	double fadeOutStart() const;
	double fadeOutDuration() const;
	double transitionDuration() const;
	int    durationMs() const { return std::max<int>(1, std::max(fadeInDuration(), fadeOutStart() + fadeOutDuration()) * 1000); }

	double progress() const;

	bool synchronize() const;
	bool repeat() const;

	int timeOfDayMs() const;

private:
	QTimer  *                 m_frameTimer;
	std::chrono::milliseconds m_startTime;

	QFileInfo				  m_texturePath[2];

	bool loadFile(QImage & newImage, const QString &fileName);
	void saveGIF();

	void loadTexture(int, const QString &);
	void reloadTexture(int);
	void clearTexture(int);
	void updateUI();

	void update();
	void play();


	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
