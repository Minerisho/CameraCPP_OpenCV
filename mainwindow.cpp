#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QInputDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QAction>
#include <QSettings> 

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(new QTimer(this)),
    isRecording(false),
    selectedCameraIndex(0)
{
    ui->setupUi(this);
    ui->videoLabel->setAlignment(Qt::AlignCenter);

    loadSettings(); // Cargar configuración guardada 

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

void MainWindow::loadSettings()
{
    QSettings settings("PredictPath UIS", "Histomerge");
    // Cargar índice de cámara (si existe, si no, usa el default 0)
    selectedCameraIndex = settings.value("selectedCameraIndex", 0).toInt();
    QString loadedPath = settings.value("saveFolderPath").toString();
    if (!loadedPath.isEmpty() && QDir(loadedPath).exists()) {
        saveFolderPath = loadedPath;
        qInfo() << "Carpeta de guardado cargada:" << saveFolderPath;
    } else {
        qInfo() << "No se encontró ruta guardada o es inválida. Usando ruta por defecto.";
        setupDefaultSavePath(); 
    }

}

void MainWindow::saveSettings()
{
    QSettings settings("HistomergeDev", "HistomergeCameraApp");
    settings.setValue("selectedCameraIndex", selectedCameraIndex);
    settings.setValue("saveFolderPath", saveFolderPath);
     qInfo() << "Configuración guardada.";
}


void MainWindow::setupDefaultSavePath()
{
    QString appDir = QApplication::applicationDirPath();
    QDir dir(appDir);
    dir.cdUp();
    saveFolderPath = dir.absoluteFilePath("capturas");
    QDir captureDir(saveFolderPath);

    if (!captureDir.exists()) {
        if (!captureDir.mkpath(".")) {
             qWarning() << "Error: No se pudo crear el directorio por defecto:" << saveFolderPath;
             saveFolderPath = QApplication::applicationDirPath();
        }
    }
     qInfo() << "Establecida carpeta de guardado por defecto:" << saveFolderPath;
}


void MainWindow::initializeCamera()
{
    if(cap.isOpened()){
        cap.release();
    }
    if(timer->isActive()){
        timer->stop();
    }

    cap.open(selectedCameraIndex, cv::CAP_ANY);

    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Error de Cámara", QString("No se pudo abrir la cámara con índice %1.").arg(selectedCameraIndex));
        ui->recordButton->setEnabled(false);
        ui->captureButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
    } else {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920.0);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080.0);

        double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        qInfo() << "Iniciando cámara índice:" << selectedCameraIndex;
        qInfo() << "Resolución obtenida:" << actualWidth << "x" << actualHeight;

        timer->start(30);
    }
}

void MainWindow::updateFrame()
{
    if (!cap.isOpened()) return;

    cap >> frame;

    if (frame.empty()) {
        return;
    }

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
    ui->videoLabel->setPixmap(QPixmap::fromImage(qtImage).scaled(ui->videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    updateButtonStates();
}


void MainWindow::on_recordButton_clicked()
{
    if (isRecording || !cap.isOpened()) return;

    QString baseName = QString("VID%1.avi").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QDir saveDir(saveFolderPath);
    QString filename = saveDir.filePath(baseName);

     if (!saveDir.exists()) {
        QMessageBox::critical(this, "Error de Grabación", QString("La carpeta de guardado no existe:\n%1").arg(saveFolderPath));
        return;
    }


    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = 15.0;
    if (fps <= 0) fps = 15.0;

    bool opened = writer.open(filename.toStdString(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(frameWidth, frameHeight));

    if (!opened) {
        QMessageBox::critical(this, "Error de Grabación", QString("No se pudo abrir el archivo para grabar:\n%1").arg(filename));
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
    if (!cap.isOpened() || flippedFrame.empty()) {
        QMessageBox::warning(this, "Captura", "No hay imagen válida para capturar o la cámara no está abierta.");
        return;
    }

    QString baseName = QString("IMG%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QDir saveDir(saveFolderPath);
    QString filename = saveDir.filePath(baseName);

    if (!saveDir.exists()) {
        QMessageBox::critical(this, "Error de Captura", QString("La carpeta de guardado no existe:\n%1").arg(saveFolderPath));
        return;
    }

    bool saved = cv::imwrite(filename.toStdString(), flippedFrame);

    if (saved) {
        qInfo() << "Imagen guardada ->" << filename;
    } else {
        QMessageBox::critical(this, "Error de Captura", "No se pudo guardar la imagen.");
    }
}

void MainWindow::on_exitButton_clicked()
{
    saveSettings();
    close();
}


void MainWindow::updateButtonStates()
{
    bool cameraIsOpen = cap.isOpened();
    bool frameIsValid = !flippedFrame.empty();
    bool enableButtons = cameraIsOpen && frameIsValid;

    ui->recordButton->setEnabled(enableButtons && !isRecording);
    ui->stopButton->setEnabled(enableButtons && isRecording);
    ui->captureButton->setEnabled(enableButtons);
    ui->exitButton->setEnabled(true);
}


void MainWindow::on_actionSeleccionar_carpeta_de_guardado_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Seleccionar Carpeta de Guardado",
                                                    saveFolderPath,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        saveFolderPath = dir;
        qInfo() << "Nueva carpeta de guardado seleccionada:" << saveFolderPath;
        saveSettings(); 
    }
}

void MainWindow::on_actionSeleccionar_camara_triggered()
{
    bool ok;
    int newIndex = QInputDialog::getInt(this,
                                        "Seleccionar Cámara",
                                        "Índice de Cámara (0, 1, 2...):",
                                        selectedCameraIndex,
                                        0,
                                        10,
                                        1,
                                        &ok);

    if (ok && newIndex != selectedCameraIndex) {
        selectedCameraIndex = newIndex;
        qInfo() << "Intentando cambiar al índice de cámara:" << selectedCameraIndex;
        saveSettings(); 
        if(isRecording){
            on_stopButton_clicked();
        }
        frame = cv::Mat();
        flippedFrame = cv::Mat();
        ui->videoLabel->clear(); 
        QApplication::processEvents(); 
        initializeCamera();
    }
}