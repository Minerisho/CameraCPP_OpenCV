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
#include <QKeySequence> // Necesario
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(new QTimer(this)),
    isRecording(false),
    selectedCameraIndex(0)
{
    ui->setupUi(this);
    ui->videoLabel->setAlignment(Qt::AlignCenter);

    loadSettings(); // Carga carpeta, cámara, formato Y ATAJOS
    applyShortcuts(); // Aplica los atajos cargados a los botones

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

    // Cargar Atajos (con defaults si no existen)
    m_shortcutRecord = settings.value("shortcutRecord", QKeySequence("Ctrl+G")).value<QKeySequence>();
    m_shortcutStop = settings.value("shortcutStop", QKeySequence("Ctrl+D")).value<QKeySequence>();
    m_shortcutCapture = settings.value("shortcutCapture", QKeySequence("Ctrl+C")).value<QKeySequence>();
    qInfo() << "Atajos cargados - Grabar:" << m_shortcutRecord.toString()
            << "Detener:" << m_shortcutStop.toString()
            << "Capturar:" << m_shortcutCapture.toString();
}

void MainWindow::saveSettings()
{
    QSettings settings("HistomergeDev", "HistomergeCameraApp");
    settings.setValue("selectedCameraIndex", selectedCameraIndex);
    settings.setValue("saveFolderPath", saveFolderPath);
    settings.setValue("selectedFourcc", selectedFourcc);
    settings.setValue("selectedExtension", selectedExtension);
    // Guardar Atajos
    settings.setValue("shortcutRecord", m_shortcutRecord);
    settings.setValue("shortcutStop", m_shortcutStop);
    settings.setValue("shortcutCapture", m_shortcutCapture);

    qInfo() << "Configuración guardada.";
}

// Nueva función para aplicar los atajos a los botones
void MainWindow::applyShortcuts()
{
    ui->recordButton->setShortcut(m_shortcutRecord);
    ui->stopButton->setShortcut(m_shortcutStop);
    ui->captureButton->setShortcut(m_shortcutCapture);
    // Opcional: podrías asignar atajos a las QAction del menú también
    // ui->actionGrabar->setShortcut(m_shortcutRecord); // Si tuvieras acciones para ellos
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

    frame = cv::Mat();
    flippedFrame = cv::Mat();
    ui->videoLabel->clear();
    QApplication::processEvents();

    cap.open(selectedCameraIndex, cv::CAP_ANY);

    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Error de Cámara", QString("No se pudo abrir la cámara con índice %1.").arg(selectedCameraIndex));
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
         QMessageBox::warning(this, "Grabar", "La cámara no está lista o no hay frames válidos.");
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
        QMessageBox::critical(this, "Error de Grabación", QString("No se pudo abrir el archivo para grabar (¿Códec seleccionado compatible e instalado?):\n%1").arg(filename));
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
    // Habilitar/deshabilitar nuevas acciones de menú si es necesario
    ui->actionAtajos_de_teclado->setEnabled(true);
    ui->actionAcerca_de->setEnabled(true);

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
        initializeCamera();
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

// Modificado para manejar el diálogo de atajos configurable
void MainWindow::on_actionAtajos_de_teclado_triggered()
{
    // Crear diálogo pasando los atajos actuales
    ShortcutsDialog shortcutsDialog(m_shortcutRecord, m_shortcutStop, m_shortcutCapture, this);
    if (shortcutsDialog.exec() == QDialog::Accepted) {
        // Obtener nuevos atajos del diálogo
        QKeySequence newRecordSeq = shortcutsDialog.getRecordSequence();
        QKeySequence newStopSeq = shortcutsDialog.getStopSequence();
        QKeySequence newCaptureSeq = shortcutsDialog.getCaptureSequence();

        // Actualizar si cambiaron
        bool changed = false;
        if (newRecordSeq != m_shortcutRecord) {
            m_shortcutRecord = newRecordSeq;
            changed = true;
        }
        if (newStopSeq != m_shortcutStop) {
            m_shortcutStop = newStopSeq;
            changed = true;
        }
        if (newCaptureSeq != m_shortcutCapture) {
            m_shortcutCapture = newCaptureSeq;
            changed = true;
        }

        // Si hubo cambios, aplicar y guardar
        if (changed) {
            applyShortcuts(); // Aplicar a los botones
            saveSettings();   // Guardar en configuración
            qInfo() << "Atajos actualizados.";
        }
    }
}

void MainWindow::on_actionAcerca_de_triggered()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}