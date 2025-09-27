#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

    void setProgress(int value);  // Assurez-vous que cette méthode est déclarée

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
