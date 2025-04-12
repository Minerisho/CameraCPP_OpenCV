#include "formatdialog.h"
#include "ui_formatdialog.h"
#include <opencv2/videoio.hpp> // Para cv::VideoWriter::fourcc

FormatDialog::FormatDialog(int currentFourcc, const QString &currentExtension, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormatDialog)
{
    ui->setupUi(this);

    populateCodecs();
    populateContainers();
    selectInitialValues(currentFourcc, currentExtension);

    // Conectar señal para cambio de codec para sugerir extensión
    connect(ui->codecComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormatDialog::onCodecChanged);
}

FormatDialog::~FormatDialog()
{
    delete ui;
}

void FormatDialog::populateCodecs()
{
    // Añadir los códecs solicitados al mapa y al ComboBox
    // El texto es lo que ve el usuario, el QVariant almacena el FOURCC int
    codecMap.clear();
    codecMap.insert("MJPEG", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    codecMap.insert("XVID", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'));
    codecMap.insert("DIVX", cv::VideoWriter::fourcc('D', 'I', 'V', 'X'));
    codecMap.insert("MP4V (MPEG-4)", cv::VideoWriter::fourcc('M', 'P', '4', 'V'));
    codecMap.insert("H.264 (libx264)", cv::VideoWriter::fourcc('X', '2', '6', '4')); // Requiere códec x264
    codecMap.insert("H.264 (OpenH264/AVC)", cv::VideoWriter::fourcc('H', '2', '6', '4')); // Nombres alternativos
    // codecMap.insert("H.264 (avc1)", cv::VideoWriter::fourcc('a', 'v', 'c', '1')); // Otro más
    codecMap.insert("WMV1", cv::VideoWriter::fourcc('W', 'M', 'V', '1'));
    codecMap.insert("WMV2", cv::VideoWriter::fourcc('W', 'M', 'V', '2'));
    codecMap.insert("IYUV (Uncompressed)", cv::VideoWriter::fourcc('I', 'Y', 'U', 'V')); // Muy grande!
    codecMap.insert("YUY2 (Uncompressed)", cv::VideoWriter::fourcc('Y', 'U', 'Y', '2')); // Muy grande!

    // Mapa de sugerencias de extensión
    defaultExtensionMap.clear();
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), ".avi");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), ".avi");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), ".avi");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('M', 'P', '4', 'V'), ".mp4");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('X', '2', '6', '4'), ".mp4");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('H', '2', '6', '4'), ".mp4");
    // defaultExtensionMap.insert(cv::VideoWriter::fourcc('a', 'v', 'c', '1'), ".mp4");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('W', 'M', 'V', '1'), ".wmv");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('W', 'M', 'V', '2'), ".wmv");
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('I', 'Y', 'U', 'V'), ".avi"); // Sin compresión usualmente en AVI
    defaultExtensionMap.insert(cv::VideoWriter::fourcc('Y', 'U', 'Y', '2'), ".avi"); // Sin compresión usualmente en AVI


    ui->codecComboBox->clear();
    for(const QString &name : codecMap.keys()) {
        ui->codecComboBox->addItem(name, codecMap.value(name));
    }
}

void FormatDialog::populateContainers()
{
    ui->containerComboBox->clear();
    // Añadir contenedores comunes. El texto es también la extensión.
    ui->containerComboBox->addItem(".mp4", ".mp4");
    ui->containerComboBox->addItem(".avi", ".avi");
    ui->containerComboBox->addItem(".mkv", ".mkv");
    // Añadir .wmv si se quiere
     ui->containerComboBox->addItem(".wmv", ".wmv");
}

 void FormatDialog::selectInitialValues(int initialFourcc, const QString &initialExtension)
 {
     // Seleccionar codec inicial
     int index = ui->codecComboBox->findData(initialFourcc);
     if (index != -1) {
         ui->codecComboBox->setCurrentIndex(index);
     } else {
         // Si el fourcc guardado no está en la lista, seleccionar el primero (MJPEG)
          ui->codecComboBox->setCurrentIndex(0);
     }

     // Seleccionar extensión inicial
     index = ui->containerComboBox->findData(initialExtension);
      if (index != -1) {
         ui->containerComboBox->setCurrentIndex(index);
     } else {
         // Si la extensión guardada no está, seleccionar la sugerida para el codec actual
         onCodecChanged(ui->codecComboBox->currentIndex()); // Llama al slot para poner la sugerida
         // O seleccionar .mp4 por defecto si falla todo
         index = ui->containerComboBox->findData(".mp4");
         if (index != -1) ui->containerComboBox->setCurrentIndex(index); else ui->containerComboBox->setCurrentIndex(0);

     }
 }


void FormatDialog::onCodecChanged(int index)
{
    if (index < 0) return;

    int selectedFourcc = ui->codecComboBox->itemData(index).toInt();
    if (defaultExtensionMap.contains(selectedFourcc)) {
        QString suggestedExtension = defaultExtensionMap.value(selectedFourcc);
        int extIndex = ui->containerComboBox->findData(suggestedExtension);
        if (extIndex != -1) {
            ui->containerComboBox->setCurrentIndex(extIndex);
        }
    }
}


int FormatDialog::getSelectedFourcc() const
{
    // Devuelve el FOURCC int almacenado en el item seleccionado
    return ui->codecComboBox->currentData().toInt();
}

QString FormatDialog::getSelectedExtension() const
{
    // Devuelve la extensión (ej. ".mp4") almacenada en el item seleccionado
    return ui->containerComboBox->currentData().toString();
}