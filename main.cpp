// main.cpp
#include "mainwindow.h" // Incluye la definición de tu ventana principal
#include <QApplication> // Incluye la clase base de la aplicación Qt

int main(int argc, char *argv[])
{
    // 1. Crea el objeto QApplication. Necesario para cualquier aplicación Qt GUI.
    //    Gestiona recursos globales, el bucle de eventos, argumentos de línea de comandos, etc.
    QApplication app(argc, argv);

    // 2. Crea una instancia de tu ventana principal.
    MainWindow mainWindow;

    // 3. Muestra la ventana principal.
    mainWindow.show();

    // 4. Inicia el bucle de eventos de Qt.
    //    La aplicación esperará aquí eventos (clics, teclas, timeouts de timers)
    //    y los despachará a los objetos correspondientes (ventanas, widgets).
    //    La función exec() devuelve un código de salida cuando la aplicación termina
    //    (usualmente cuando se cierra la última ventana o se llama a QApplication::quit()).
    return app.exec();
}