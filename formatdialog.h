#ifndef FORMATDIALOG_H
#define FORMATDIALOG_H

#include <QDialog>
#include <QString>
#include <QMap> // Para asociar nombres con FOURCC

namespace Ui {
class FormatDialog;
}

class FormatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormatDialog(int currentFourcc, const QString &currentExtension, QWidget *parent = nullptr);
    ~FormatDialog() override;

    int getSelectedFourcc() const;
    QString getSelectedExtension() const;

private slots:
    void onCodecChanged(int index); // Slot para sugerir extensión

private:
    Ui::FormatDialog *ui;
    QMap<QString, int> codecMap; // Mapa de Nombre de Codec a FOURCC
    QMap<int, QString> defaultExtensionMap; // Mapa de FOURCC a extensión sugerida

    void populateCodecs();
    void populateContainers();
    void selectInitialValues(int initialFourcc, const QString &initialExtension);
};

#endif // FORMATDIALOG_H