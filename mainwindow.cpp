#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>

QString getSavePath(const QString& baseName) {
    QString appDir = QApplication::applicationDirPath();
    QDir dir(appDir);
    dir.cdUp();
    QString captureDirPath = dir.absoluteFilePath("capturas");
    QDir captureDir(captureDirPath);

    if (!captureDir.exists()) {
        if (!captureDir.mkpath(".")) {
             qWarning() << "Error: No se pudo crear el directorio:" << captureDirPath;
             return QApplication::applicationDirPath() + "/" + baseName;
        }
    }
    return captureDir.absoluteFilePath(baseName);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(new QTimer(this)),
    isRecording(false)
{
    ui->setupUi(this);
    ui->videoLabel->setAlignment(Qt::AlignCenter);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    initializeCamera();

    if (cap.isOpened()) {
        ui->recordButton->setEnabled(true);
        ui->captureButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
    } else {
        ui->recordButton->setEnabled(false);
        ui->captureButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
    }
    ui->exitButton->setEnabled(true);
}

MainWindow::~MainWindow()
{
    if (cap.isOpened()) {
        cap.release();
    }
    if (writer.isOpened()) {
        writer.release();
    }
    delete ui;
}

void MainWindow::initializeCamera()
{
    cap.open(0, cv::CAP_ANY);

    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Error de Cámara", "No se pudo abrir la cámara web.");
    } else {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920.0);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080.0);

        double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        qInfo() << "Resolución solicitada: 1920x1080";
        qInfo() << "Resolución obtenida:" << actualWidth << "x" << actualHeight;

        timer->start(30);
    }
}

void MainWindow::updateFrame()
{
    cap >> frame;

    if (frame.empty()) {
        timer->stop();
        if (cap.isOpened()) { // Mostrar advertencia solo si la cámara estaba supuestamente abierta
             QMessageBox::warning(this, "Error de Cámara", "Se perdió la conexión o no se reciben frames.");
             cap.release(); // Liberar explícitamente si falla la lectura
        }
        updateButtonStates();
        return;
    }
    cv::Mat flippedFrame;
    cv::flip(frame, flippedFrame, -1);
    cv::Mat frameToDisplay = flippedFrame.clone();

    if (isRecording) {
        cv::circle(frameToDisplay, cv::Point(30, 30), 10, cv::Scalar(0, 0, 255), cv::FILLED);
        cv::putText(frameToDisplay, "REC", cv::Point(50, 37), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

        if (writer.isOpened()) {
            writer.write(flippedFrame);
        } else {
             qWarning() << "Error: Intentando escribir con VideoWriter cerrado mientras isRecording es true.";
             isRecording = false;
        }
    }

    cv::Mat frameRgb;
    cv::cvtColor(frameToDisplay, frameRgb, cv::COLOR_BGR2RGB);
    QImage qtImage(frameRgb.data, frameRgb.cols, frameRgb.rows, frameRgb.step, QImage::Format_RGB888);
    ui->videoLabel->setPixmap(QPixmap::fromImage(qtImage));

    updateButtonStates();
}


void MainWindow::on_recordButton_clicked()
{
    if (isRecording || !cap.isOpened()) return;

    QString baseName = QString("VID%1.avi").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QString filename = getSavePath(baseName);

    if (filename.isEmpty()) {
         QMessageBox::critical(this, "Error de Grabación", "No se pudo determinar la ruta de guardado.");
         return;
    }

    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0) fps = 30.0;

    bool opened = writer.open(filename.toStdString(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(frameWidth, frameHeight));

    if (!opened) {
        QMessageBox::critical(this, "Error de Grabación", QString("No se pudo abrir el archivo para grabar:\n%1\nAsegúrate de tener códecs compatibles (ej. MP4V) instalados.").arg(filename));
        return;
    }

    isRecording = true;
    qInfo() << "Grabación iniciada ->" << filename;
    updateButtonStates();
}

void MainWindow::on_stopButton_clicked()
{
    if (!isRecording) return;

    isRecording = false;
    if (writer.isOpened()) {
        writer.release();
        qInfo() << "Grabación detenida.";
    }
    updateButtonStates();
}

void MainWindow::on_captureButton_clicked()
{
    if (!cap.isOpened() || frame.empty()) { // Añadida comprobación extra
        QMessageBox::warning(this, "Captura", "No hay imagen válida para capturar o la cámara no está abierta.");
        return;
    }

    QString baseName = QString("IMG%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QString filename = getSavePath(baseName);

     if (filename.isEmpty()) {
         QMessageBox::critical(this, "Error de Captura", "No se pudo determinar la ruta de guardado.");
         return;
    }

    bool saved = cv::imwrite(filename.toStdString(), frame);

    if (saved) {
        qInfo() << "Imagen guardada ->" << filename;
    } else {
        QMessageBox::critical(this, "Error de Captura", "No se pudo guardar la imagen.");
    }
}

void MainWindow::on_exitButton_clicked()
{
    close();
}


void MainWindow::updateButtonStates()
{
    bool cameraIsOpen = cap.isOpened(); // Solo verificar si la cámara está abierta

    ui->recordButton->setEnabled(cameraIsOpen && !isRecording);
    ui->stopButton->setEnabled(cameraIsOpen && isRecording);
    ui->captureButton->setEnabled(cameraIsOpen); // Habilitar si la cámara está abierta
    ui->exitButton->setEnabled(true);
}