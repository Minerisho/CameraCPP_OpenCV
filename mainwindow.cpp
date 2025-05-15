#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formatdialog.h"
#include "shortcutsdialog.h"
#include "aboutdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QInputDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QAction>
#include <QSettings>
#include <QDebug>
#include <QKeySequence>
#include <QPushButton>
#include <QShowEvent> 

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(new QTimer(this)),
    isRecording(false),
    selectedCameraIndex(0),
    initialCameraFailed(false) 
{
    ui->setupUi(this);
    ui->videoLabel->setAlignment(Qt::AlignCenter);
    loadSettings();
    applyShortcuts();
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    initializeCamera();
    updateButtonStates();
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

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (initialCameraFailed) {
        QMessageBox::critical(this, "Error de Cámara",
                             QString("No se pudo abrir la cámara con índice %1.").arg(selectedCameraIndex));
        initialCameraFailed = false;
    }
}



void MainWindow::loadSettings()
{
     QSettings settings("HistomergeDev", "HistomergeCameraApp");
    selectedCameraIndex = settings.value("selectedCameraIndex", 0).toInt();
    QString loadedPath = settings.value("saveFolderPath").toString();
    if (!loadedPath.isEmpty() && QDir(loadedPath).exists()) {
        saveFolderPath = loadedPath;
        qInfo() << "Carpeta de guardado cargada:" << saveFolderPath;
    } else {
        qInfo() << "No se encontró ruta guardada o es inválida. Usando ruta por defecto.";
        setupDefaultSavePath();
    }
    selectedFourcc = settings.value("selectedFourcc", cv::VideoWriter::fourcc('M', 'J', 'P', 'G')).toInt();
    selectedExtension = settings.value("selectedExtension", ".avi").toString();
    qInfo() << "Formato cargado - FOURCC:" << selectedFourcc << "Extensión:" << selectedExtension;

    recordShortcut = settings.value("shortcut_record", QKeySequence("Ctrl+G")).value<QKeySequence>();
    stopShortcut = settings.value("shortcut_stop", QKeySequence("Ctrl+D")).value<QKeySequence>();
    captureShortcut = settings.value("shortcut_capture", QKeySequence("Ctrl+C")).value<QKeySequence>();
    qInfo() << "Atajos cargados - Grabar:" << recordShortcut.toString() << "Detener:" << stopShortcut.toString() << "Capturar:" << captureShortcut.toString();
}

void MainWindow::saveSettings()
{
    
    QSettings settings("HistomergeDev", "HistomergeCameraApp");
    settings.setValue("selectedCameraIndex", selectedCameraIndex);
    settings.setValue("saveFolderPath", saveFolderPath);
    settings.setValue("selectedFourcc", selectedFourcc);
    settings.setValue("selectedExtension", selectedExtension);
    settings.setValue("shortcut_record", recordShortcut);
    settings.setValue("shortcut_stop", stopShortcut);
    settings.setValue("shortcut_capture", captureShortcut);
    qInfo() << "Configuración guardada.";
}

void MainWindow::applyShortcuts()
{
   
    ui->recordButton->setShortcut(recordShortcut);
    ui->stopButton->setShortcut(stopShortcut);
    ui->captureButton->setShortcut(captureShortcut);
    ui->recordButton->setToolTip(QString("Grabar (%1)").arg(recordShortcut.toString(QKeySequence::NativeText)));
    ui->stopButton->setToolTip(QString("Detener (%1)").arg(stopShortcut.toString(QKeySequence::NativeText)));
    ui->captureButton->setToolTip(QString("Capturar Frame (%1)").arg(captureShortcut.toString(QKeySequence::NativeText)));
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
    initialCameraFailed = false;

    if(cap.isOpened()){
        cap.release();
    }
    if(timer->isActive()){
        timer->stop();
    }

    frame = cv::Mat();
    flippedFrame = cv::Mat();
    ui->videoLabel->clear();
    QApplication::processEvents();

    cap.open(selectedCameraIndex, cv::CAP_ANY);

    if (!cap.isOpened()) {

        qWarning() << "Fallo al abrir cámara índice:" << selectedCameraIndex;
        initialCameraFailed = true; 
        updateButtonStates();       
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
     if (!cap.isOpened()) {
        frame = cv::Mat();
        flippedFrame = cv::Mat();
         if (!ui->videoLabel->pixmap(Qt::ReturnByValue).isNull()) {
             ui->videoLabel->clear();
        }
        if (isRecording) {
            on_stopButton_clicked();
        }
        updateButtonStates();
        return;
     }

    cap >> frame;

    if (frame.empty()) {
       
        ui->videoLabel->clear(); 
        updateButtonStates(); 
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
             updateButtonStates();
        }
    }

    cv::Mat frameRgb;
    cv::cvtColor(frameToDisplay, frameRgb, cv::COLOR_BGR2RGB);
    QImage qtImage(frameRgb.data, frameRgb.cols, frameRgb.rows, frameRgb.step, QImage::Format_RGB888);
     if (!qtImage.isNull()) {
        ui->videoLabel->setPixmap(QPixmap::fromImage(qtImage).scaled(ui->videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    updateButtonStates();
}

void MainWindow::on_recordButton_clicked()
{
    if (isRecording || !cap.isOpened() || flippedFrame.empty()) {
         if (!cap.isOpened()) QMessageBox::warning(this, "Grabar", "La cámara no está abierta.");
         else if (flippedFrame.empty()) QMessageBox::warning(this, "Grabar", "Esperando frame válido...");
        return;
    }

    QString baseName = QString("VID%1%2").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss")).arg(selectedExtension);
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

    bool opened = writer.open(filename.toStdString(), selectedFourcc, fps, cv::Size(frameWidth, frameHeight));

    if (!opened) {
            char fourcc_str[5] = {0};
            fourcc_str[0] = (selectedFourcc >> 0) & 0xFF;
            fourcc_str[1] = (selectedFourcc >> 8) & 0xFF;
            fourcc_str[2] = (selectedFourcc >> 16) & 0xFF;
            fourcc_str[3] = (selectedFourcc >> 24) & 0xFF;
            QString fourcc_qstr(fourcc_str);

            QMessageBox::critical(this, "Error de Grabación",
                QString("No se pudo abrir el archivo para grabar.\n"
                        "Archivo: %1\n"
                        "Códec FOURCC: %2 (0x%3)\n"
                        "¿Códec seleccionado compatible e instalado y accesible para OpenCV?")
                .arg(filename)
                .arg(fourcc_qstr)
                .arg(QString::number(selectedFourcc, 16)));
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
    bool enableActions = cameraIsOpen && frameIsValid;

    ui->recordButton->setEnabled(enableActions && !isRecording);
    ui->stopButton->setEnabled(enableActions && isRecording);
    ui->captureButton->setEnabled(enableActions);
    ui->exitButton->setEnabled(true);


    ui->actionFormato_de_salida->setEnabled(true);
    ui->actionSeleccionar_camara->setEnabled(true);
    ui->actionSeleccionar_carpeta_de_guardado->setEnabled(true);

}


void MainWindow::on_actionSeleccionar_carpeta_de_guardado_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Seleccionar Carpeta de Guardado",
                                                    saveFolderPath,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty() && dir != saveFolderPath) {
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
         if(isRecording){
            on_stopButton_clicked();
        }
        selectedCameraIndex = newIndex;
        qInfo() << "Intentando cambiar al índice de cámara:" << selectedCameraIndex;
        saveSettings();
        initializeCamera(); // initializeCamera ahora limpia y reinicia
    }
}


void MainWindow::on_actionFormato_de_salida_triggered()
{
    FormatDialog formatDialog(selectedFourcc, selectedExtension, this);
    if (formatDialog.exec() == QDialog::Accepted) {
        selectedFourcc = formatDialog.getSelectedFourcc();
        selectedExtension = formatDialog.getSelectedExtension();
        qInfo() << "Nuevo formato seleccionado - FOURCC:" << selectedFourcc << "Extensión:" << selectedExtension;
        saveSettings();
    }
}

void MainWindow::on_actionAtajos_de_teclado_triggered()
{
    ShortcutsDialog shortcutsDialog(recordShortcut, stopShortcut, captureShortcut, this);
    if (shortcutsDialog.exec() == QDialog::Accepted) {
        recordShortcut = shortcutsDialog.getRecordSequence();
        stopShortcut = shortcutsDialog.getStopSequence();
        captureShortcut = shortcutsDialog.getCaptureSequence();

        qInfo() << "Nuevos atajos guardados.";
        applyShortcuts();
        saveSettings();
    }
}

void MainWindow::on_actionAcerca_de_triggered()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}