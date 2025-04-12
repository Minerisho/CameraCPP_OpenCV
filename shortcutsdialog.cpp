#include "shortcutsdialog.h"
#include "ui_shortcutsdialog.h"
#include <QKeySequenceEdit> // Necesario

ShortcutsDialog::ShortcutsDialog(const QKeySequence &recordSeq,
                               const QKeySequence &stopSeq,
                               const QKeySequence &captureSeq,
                               QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutsDialog)
{
    ui->setupUi(this);

    // Establecer los atajos actuales en los widgets QKeySequenceEdit
    ui->atajo_grabar->setKeySequence(recordSeq);
    ui->atajo_detener->setKeySequence(stopSeq);
    ui->atajo_capturar->setKeySequence(captureSeq);
}

ShortcutsDialog::~ShortcutsDialog()
{
    delete ui;
}

QKeySequence ShortcutsDialog::getRecordSequence() const
{
    return ui->atajo_grabar->keySequence();
}

QKeySequence ShortcutsDialog::getStopSequence() const
{
    return ui->atajo_detener->keySequence();
}

QKeySequence ShortcutsDialog::getCaptureSequence() const
{
    return ui->atajo_capturar->keySequence();
}