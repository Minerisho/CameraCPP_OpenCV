#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QKeySequence>
#include <QShowEvent> // <-- Incluir para QShowEvent

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FormatDialog;
class ShortcutsDialog;
class AboutDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void updateFrame();
    void on_recordButton_clicked();
    void on_stopButton_clicked();
    void on_captureButton_clicked();
    void on_exitButton_clicked();
    void on_actionSeleccionar_carpeta_de_guardado_triggered();
    void on_actionSeleccionar_camara_triggered();
    void on_actionFormato_de_salida_triggered();
    void on_actionAtajos_de_teclado_triggered();
    void on_actionAcerca_de_triggered();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture cap;
    cv::VideoWriter writer;
    cv::Mat frame;
    cv::Mat flippedFrame;
    QTimer *timer;
    bool isRecording;
    QString saveFolderPath;
    int selectedCameraIndex;
    int selectedFourcc;
    QString selectedExtension;
    QKeySequence recordShortcut;
    QKeySequence stopShortcut;
    QKeySequence captureShortcut;

    bool initialCameraFailed;

    void initializeCamera();
    void updateButtonStates();
    void setupDefaultSavePath();
    void loadSettings();
    void saveSettings();
    void applyShortcuts();

};

#endif // MAINWINDOW_H