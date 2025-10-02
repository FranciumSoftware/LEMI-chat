#ifndef DIAGUP_H
#define DIAGUP_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "progressdialog.h" // Ajoutez cette ligne

namespace Ui {
class DiagUp;
}

class DiagUp : public QDialog {
    Q_OBJECT

public:
    explicit DiagUp(QWidget *parent = nullptr);
    ~DiagUp();

private slots:
    void on_fermer_clicked();
    void on_update_clicked();
    void onDownloadFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void on_update_pressed();

    void on_fermer_released();

    void on_update_released();

    void on_fermer_pressed();

private:
    Ui::DiagUp *ui;
    QStringList versionList;
    QStringList downloadList;
    QStringList displayList;
    QString download;
    QNetworkAccessManager *networkManager;
    QNetworkReply *currentReply;
    QString downloadFilePath;
    ProgressDialog *progressDialog; // Ajoutez cette ligne
    void checkForUpdate();
    void downloadFile(const QString &fileUrl);
};

#endif // DIAGUP_H
