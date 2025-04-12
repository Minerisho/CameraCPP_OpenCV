#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>
#include <QKeySequence> // Necesario

namespace Ui {
class ShortcutsDialog;
}

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    // El constructor ahora recibe los atajos actuales
    explicit ShortcutsDialog(const QKeySequence &recordSeq,
                               const QKeySequence &stopSeq,
                               const QKeySequence &captureSeq,
                               QWidget *parent = nullptr);
    ~ShortcutsDialog() override;

    // Getters para obtener los nuevos atajos introducidos por el usuario
    QKeySequence getRecordSequence() const;
    QKeySequence getStopSequence() const;
    QKeySequence getCaptureSequence() const;

private:
    Ui::ShortcutsDialog *ui;
};

#endif // SHORTCUTSDIALOG_H