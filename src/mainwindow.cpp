#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QTimer>

#include <ctime>

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
m_frameTimer(new QTimer(this)),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->openGLWidget->w = this;

	m_frameTimer->setInterval(16);
	m_frameTimer->setSingleShot(false);

	connect(m_frameTimer, &QTimer::timeout, this, &MainWindow::update);
	connect(ui->play, &QPushButton::clicked, this,  &MainWindow::play);
	connect(ui->ExportGif, &QPushButton::clicked, this,  &MainWindow::saveGIF);

	connect(ui->loadErosion, &QPushButton::clicked, [this]() { loadTexture(0, tr("Load Erosion")); });
	connect(ui->reloadErosion, &QPushButton::clicked, [this]() { reloadTexture(0); });
	connect(ui->clearErosion, &QPushButton::clicked, [this]() { clearTexture(0); });

	connect(ui->loadUV, &QPushButton::clicked, [this]() { loadTexture(1, tr("Load Gradient")); });
	connect(ui->reloadUV, &QPushButton::clicked, [this]() { reloadTexture(1); });
	connect(ui->clearUV, &QPushButton::clicked, [this]() { clearTexture(1); });

	connect(ui->synchronize, &QCheckBox::stateChanged, [this](int checked) { ui->progress->setEnabled(checked == Qt::Unchecked); });
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::loadTexture(int id, const QString & title)
{
	QFileDialog dialog(this, title);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	QImage      image;
    while (dialog.exec() == QDialog::Accepted && !loadFile(image, dialog.selectedFiles().first())) {}

	if(image.isNull())
	{
		return;
	}

	ui->openGLWidget->setTexture(id, image);
	m_texturePath[id] = QFileInfo(dialog.selectedFiles().first());
	updateUI();
}

void MainWindow::reloadTexture(int id)
{
	QImage image;
    loadFile(image, m_texturePath[id].filePath());

	if(!image.isNull())
		ui->openGLWidget->setTexture(id, image);
	else
	{
		ui->openGLWidget->clearTexture(id);
		m_texturePath[id] = QFileInfo();
		updateUI();
	}
}

void MainWindow::clearTexture(int id)
{
	m_texturePath[id] = QFileInfo();
	ui->openGLWidget->clearTexture(id);
	updateUI();
}

void MainWindow::updateUI()
{
	bool haveErosion = (m_texturePath[0].isFile());
	bool haveUV      = (m_texturePath[1].isFile());

	ui->erosionPath->setText(m_texturePath[0].fileName());
	ui->reloadErosion->setEnabled(haveErosion);
	ui->clearErosion-> setEnabled(haveErosion);

	ui->uvPath->setText(m_texturePath[1].fileName());
	ui->reloadUV->setEnabled(haveUV);
	ui->clearUV-> setEnabled(haveUV);

	ui->play->setEnabled(haveErosion);
	ui->ExportGif->setEnabled(haveErosion);
	ui->progress->setEnabled(haveErosion && ui->synchronize->checkState() == Qt::Unchecked);

	if(!haveErosion)
	{
		m_frameTimer->stop();
	}
	else
	{
		m_frameTimer->start();
	}
}

void MainWindow::play()
{
	if(!ui->play->isChecked())
	{
		return;
	}

	std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	int duration = durationMs();
	int progress;

	if(synchronize())
	{
		progress = ms.count() % (duration + 1);
		double value = progress * ui->progress->maximum() / (double) (duration + .01) + .5;
		ui->progress->setValue(value);
	}
	else
	{
		progress = ui->progress->value() * duration / (double) ui->progress->maximum() + .5;
	}

	m_startTime = ms - std::chrono::milliseconds(progress);
}

void MainWindow::update()
{
//get current time
	if(ui->play->isChecked())
	{
		std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
		int duration = durationMs();
		int progress;

		if(synchronize())
		{
			progress = ms.count() % (duration + 1);
		}
		else
		{
			progress = (ms - m_startTime).count();
		}

		double value = progress * ui->progress->maximum() / (double) (duration + .01) + .5;

		if(value > ui->progress->value() && value < ui->progress->maximum())
		{
			ui->progress->setValue(value);
		}
		else
		{
			ui->progress->setValue(0);
			m_startTime = ms;

			if(!repeat())
			{
				ui->play->setChecked(false);
			}
		}
	}

	ui->openGLWidget->repaint();
}

int MainWindow::timeOfDayMs() const
{
	auto now = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) -
			  std::chrono::duration_cast<std::chrono::hours>(now.time_since_epoch());
	return ms.count();
}

double MainWindow::fadeInDuration() const
{
	return ui->fadeInDuration->value();
}

double  MainWindow::fadeOutStart() const
{
	return ui->fadeOutStart->value();
}

double  MainWindow::fadeOutDuration() const
{
	return ui->fadeOutDuration->value();
}

double  MainWindow::transitionDuration() const
{
	return ui->transitionDuration->value();
}


double  MainWindow::progress() const
{
	return ui->progress->value() / (double)(ui->progress->maximum());
}

bool    MainWindow::synchronize() const
{
	return ui->synchronize->isChecked();
}

bool    MainWindow::repeat() const
{
	return ui->loop->isChecked();
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
  //      dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
		dialog.setDirectory(QDir::currentPath());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");

    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

bool  MainWindow::loadFile(QImage & newImage, const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    newImage = reader.read();

    if (!newImage.isNull())
	{
		if(newImage.width() < 2048 && newImage.height() < 2048)
			return true;

		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
								tr("Cannot load %1:\nImage too large. max size: 2048x2048")
								.arg(QDir::toNativeSeparators(fileName), reader.errorString()));

		return false;
	}

	QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
							 tr("Cannot load %1:\n%2")
							 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
	return false;
}

void MainWindow::saveGIF()
{
	if(!m_texturePath[0].isFile())
		return;

	QString filename;
	int frame_delay = 5;
	int height      = -1;

	{

		QFileDialog dialog(this, "Export GIF");
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setMimeTypeFilters({"Animated Gif (*.gif)"});
		dialog.selectMimeTypeFilter("image/gif");
		dialog.setDefaultSuffix("gif");

		if(dialog.exec() != QDialog::Accepted)
			return;

		filename = dialog.selectedFiles().first();

		if(filename.isNull())
			return;
	}

	ui->openGLWidget->ExportGif(filename.toStdString(),
		fadeInDuration(),
		fadeOutStart(),
		fadeOutDuration(),
		transitionDuration(),
		frame_delay,
		height);
}
