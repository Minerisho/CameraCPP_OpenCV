// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow> // Clase base para ventanas principales
#include <QTimer>      // Para actualizar el video periódicamente
#include <QImage>      // Para manejar imágenes en formato Qt
#include <QPixmap>     // Para mostrar imágenes en Labels

// Incluir cabeceras de OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> // Específicamente para VideoCapture y VideoWriter

// Forward declaration de la clase generada por UIC a partir de mainwindow.ui
// Esto evita tener que incluir ui_mainwindow.h aquí.
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT // ¡Macro fundamental para clases con señales y slots!

public:
    // Constructor explícito para evitar conversiones implícitas
    explicit MainWindow(QWidget *parent = nullptr);
    // Destructor virtual para correcta limpieza
    ~MainWindow() override;

private slots:
    // Slots que se conectarán a las señales de los widgets (botones, timer)
    void updateFrame();          // Slot para procesar un nuevo frame de la cámara
    void on_recordButton_clicked(); // Slot conectado automáticamente al clic de recordButton
    void on_stopButton_clicked();   // Slot conectado automáticamente al clic de stopButton
    void on_captureButton_clicked();// Slot conectado automáticamente al clic de captureButton
    void on_exitButton_clicked();   // Slot conectado automáticamente al clic de exitButton

private:
    // --- Miembros de la Clase ---
    Ui::MainWindow *ui; // Puntero a la interfaz de usuario definida en mainwindow.ui

    cv::VideoCapture cap;  // Objeto de OpenCV para capturar video de la cámara
    cv::VideoWriter writer; // Objeto de OpenCV para escribir video a archivo
    cv::Mat frame;         // Matriz de OpenCV para almacenar el frame actual
    QTimer *timer;         // Temporizador para disparar la captura de frames

    bool isRecording;      // Bandera para saber si estamos grabando

    // --- Funciones Auxiliares ---
    void initializeCamera(); // Función para configurar e iniciar la cámara
    void updateButtonStates(); // Función para habilitar/deshabilitar botones según el estado
};

#endif // MAINWINDOW_H