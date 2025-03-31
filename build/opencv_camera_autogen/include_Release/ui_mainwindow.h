/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSeleccionar_carpeta_de_guardado;
    QAction *actionSeleccionar_camara;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *videoLabel;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QPushButton *recordButton;
    QPushButton *stopButton;
    QPushButton *captureButton;
    QPushButton *exitButton;
    QMenuBar *menubar;
    QMenu *menuOpciones;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(799, 693);
        actionSeleccionar_carpeta_de_guardado = new QAction(MainWindow);
        actionSeleccionar_carpeta_de_guardado->setObjectName("actionSeleccionar_carpeta_de_guardado");
        actionSeleccionar_camara = new QAction(MainWindow);
        actionSeleccionar_camara->setObjectName("actionSeleccionar_camara");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        videoLabel = new QLabel(centralwidget);
        videoLabel->setObjectName("videoLabel");
        videoLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(videoLabel);

        frame = new QFrame(centralwidget);
        frame->setObjectName("frame");
        frame->setMaximumSize(QSize(16777215, 50));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName("horizontalLayout");
        recordButton = new QPushButton(frame);
        recordButton->setObjectName("recordButton");
        recordButton->setMaximumSize(QSize(80, 23));

        horizontalLayout->addWidget(recordButton);

        stopButton = new QPushButton(frame);
        stopButton->setObjectName("stopButton");
        stopButton->setEnabled(false);
        stopButton->setMaximumSize(QSize(80, 23));

        horizontalLayout->addWidget(stopButton);

        captureButton = new QPushButton(frame);
        captureButton->setObjectName("captureButton");
        captureButton->setMaximumSize(QSize(100, 23));

        horizontalLayout->addWidget(captureButton);

        exitButton = new QPushButton(frame);
        exitButton->setObjectName("exitButton");
        exitButton->setMaximumSize(QSize(80, 23));

        horizontalLayout->addWidget(exitButton);


        verticalLayout->addWidget(frame);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 799, 21));
        menuOpciones = new QMenu(menubar);
        menuOpciones->setObjectName("menuOpciones");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuOpciones->menuAction());
        menuOpciones->addAction(actionSeleccionar_carpeta_de_guardado);
        menuOpciones->addAction(actionSeleccionar_camara);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Histomerge", nullptr));
        actionSeleccionar_carpeta_de_guardado->setText(QCoreApplication::translate("MainWindow", "Seleccionar carpeta de guardado", nullptr));
        actionSeleccionar_camara->setText(QCoreApplication::translate("MainWindow", "Seleccionar c\303\241mara", nullptr));
        videoLabel->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
#if QT_CONFIG(accessibility)
        recordButton->setAccessibleName(QCoreApplication::translate("MainWindow", "Grabar", nullptr));
#endif // QT_CONFIG(accessibility)
        recordButton->setText(QCoreApplication::translate("MainWindow", "Grabar", nullptr));
        stopButton->setText(QCoreApplication::translate("MainWindow", "Detener", nullptr));
        captureButton->setText(QCoreApplication::translate("MainWindow", "Capturar Frame", nullptr));
        exitButton->setText(QCoreApplication::translate("MainWindow", "Salir", nullptr));
        menuOpciones->setTitle(QCoreApplication::translate("MainWindow", "Opciones", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
