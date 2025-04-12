#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QPixmap>
#include <QDebug>
#include <QSizePolicy>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QPixmap uisPixmap(":/uis_logo.png");
    if (!uisPixmap.isNull()) {
        ui->uisLogoLabel->setPixmap(uisPixmap);
    } else {
        ui->uisLogoLabel->setText("UIS Logo\n(No encontrado)");
        qWarning() << "No se pudo cargar el recurso :/uis_logo.png";
    }

    QPixmap ppPixmap(":/predictpath_logo.png");
     if (!ppPixmap.isNull()) {
        ui->predictpathLogoLabel->setPixmap(ppPixmap);
    } else {
        // Si el label del logo de predictpath aún existe en el UI, mostrar error
        // Si lo eliminaste del UI, puedes borrar esta sección 'else'.
        ui->predictpathLogoLabel->setText("PredictPath Logo\n(No encontrado)");
         qWarning() << "No se pudo cargar el recurso :/predictpath_logo.png";
    }

    // El texto ahora parece estar mayormente definido en el .ui
    // Asegurarse de que los links funcionen (ya configurado en el .ui)
    ui->infoLabel->setTextFormat(Qt::RichText);
    ui->infoLabel->setOpenExternalLinks(true);

    // Ajustar tamaño
    ui->infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // ui->authorLabel ya no existe
    adjustSize();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}