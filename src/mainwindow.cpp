#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QTimer>

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

	connect(ui->loadErosion, &QPushButton::clicked, [this]() { loadTexture(0, tr("Load Erosion")); });
	connect(ui->reloadErosion, &QPushButton::clicked, [this]() { reloadTexture(0); });
	connect(ui->clearErosion, &QPushButton::clicked, [this]() { clearTexture(0); });

	connect(ui->loadUV, &QPushButton::clicked, [this]() { loadTexture(1, tr("Load UV")); });
	connect(ui->reloadUV, &QPushButton::clicked, [this]() { reloadTexture(1); });
	connect(ui->clearUV, &QPushButton::clicked, [this]() { clearTexture(1); });

	connect(ui->progress, &QSlider::valueChanged, [this]() { ui->openGLWidget->repaint(); });
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::loadTexture(int id, const QString & title)
{
	QFileDialog dialog(this, title);
	dialog.setFileMode(QFileDialog::ExistingFile);
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
	ui->progress->setEnabled(haveErosion);

	if(!haveErosion)
	{
		m_frameTimer->stop();
	}
}

void MainWindow::play()
{
	if(!ui->play->isChecked())
	{
		m_frameTimer->stop();
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
	m_frameTimer->start();
}

void MainWindow::update()
{
//get current time
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
			m_frameTimer->stop();
			ui->play->setChecked(false);
		}
	}
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
	dialog.setDirectory(QDir::currentPath());
	/*
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }*/

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
    if (!newImage.isNull()) return true;

	QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
							 tr("Cannot load %1: %2")
							 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
	return false;
}

