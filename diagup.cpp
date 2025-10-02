#include "diagup.h"
#include "ui_diagup.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QUrl>
#include <QProcess>
#include <QGraphicsDropShadowEffect>


DiagUp::DiagUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiagUp),
    networkManager(new QNetworkAccessManager(this)),
    currentReply(nullptr),
    progressDialog(new ProgressDialog(this))
{
    ui->setupUi(this);


    checkForUpdate();
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(3, 3); // Décalage de l'ombre (x, y)
    ui->update->setGraphicsEffect(shadowCheck);
    ui->fermer->setGraphicsEffect(shadowCheck);
}

bool DiagUp::connectToMySQL() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("d6xnds.h.filess.io");
    db.setDatabaseName("LEMIMain_vastengine");
    db.setUserName("LEMIMain_vastengine");
    db.setPassword("fe55834a73cdd149f1ad5ef5c686a286d920902f");
    db.setPort(3007);

    if (!db.open()) {
        qDebug() << "Erreur de connexion à MySQL : " << db.lastError().text();
        return false;
    }
    return true;
}

void DiagUp::checkForUpdate() {
    QSqlQuery query("SELECT version, download, display FROM `update`");

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'exécution de la requête : " << query.lastError().text();
        return;
    }

    if (!query.next()) {
        qDebug() << "Aucun enregistrement trouvé dans la table 'update'.";
        return;
    }

    int version = query.value("version").toInt();
    download = query.value("download").toString().trimmed();
    QString display = query.value("display").toString().trimmed();

    const int actVer = 11;
    const QString displayVer = "1.1";

    if (version > actVer) {
        ui->message->setPlainText("Mise à jour disponible\n"
                                  "Version actuelle: " + displayVer +
                                  "\nNouvelle Version: " + display);
        ui->update->setEnabled(true);
    } else {
        ui->message->setPlainText("Aucune mise à jour disponible\n"
                                  "Version actuelle: " + displayVer);
    }
}

void DiagUp::on_fermer_clicked() {
    this->hide();
}

void DiagUp::on_update_clicked() {
    if (download.isEmpty()) {
        QMessageBox::critical(this, "Erreur", "URL de téléchargement invalide.");
        return;
    }
    downloadFile(download);
}

void DiagUp::downloadFile(const QString &fileUrl) {
    qDebug() << "Tentative de téléchargement depuis : " << fileUrl;

    if (fileUrl.isEmpty()) {
        qDebug() << "Erreur : URL vide.";
        QMessageBox::critical(this, "Erreur", "L'URL de téléchargement est vide.");
        return;
    }

    QString tempDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDirPath.isEmpty()) {
        qDebug() << "Erreur : Impossible d'obtenir le chemin du dossier temporaire.";
        QMessageBox::critical(this, "Erreur", "Impossible d'obtenir le chemin du dossier temporaire.");
        return;
    }

    QString fileName = QUrl(fileUrl).fileName();
    if (fileName.isEmpty()) {
        qDebug() << "Erreur : Impossible d'extraire le nom du fichier de l'URL.";
        QMessageBox::critical(this, "Erreur", "Impossible d'extraire le nom du fichier de l'URL.");
        return;
    }

    downloadFilePath = tempDirPath + "/" + fileName;

    QNetworkRequest request(fileUrl);
    currentReply = networkManager->get(request);

    connect(currentReply, &QNetworkReply::finished, this, &DiagUp::onDownloadFinished);
    connect(currentReply, &QNetworkReply::downloadProgress, this, &DiagUp::onDownloadProgress);

    progressDialog->setProgress(0);  // Utilisez setProgress au lieu de setValue
    progressDialog->show();
}

void DiagUp::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        progressDialog->setProgress(progress);  // Utilisez setProgress
    }
}


void DiagUp::onDownloadFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "Erreur : Réponse réseau invalide.";
        QMessageBox::warning(this,"Nous avons pas réussi à télécharger la mise à jour","Réponse réseau invalide");
        progressDialog->hide();
        return;
    }

    progressDialog->setProgress(100);  // Utilisation de setProgress



    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Erreur de téléchargement : " << reply->errorString();
        QMessageBox::critical(this, "Erreur", "Téléchargement échoué : " + reply->errorString());
        reply->deleteLater();
        currentReply = nullptr;
        progressDialog->hide();
        return;
    }

    QByteArray data = reply->readAll();

    if (data.isEmpty()) {
        qDebug() << "Erreur : Aucune donnée reçue.";
        QMessageBox::critical(this, "Erreur", "Aucune donnée reçue depuis le serveur.");
        reply->deleteLater();
        currentReply = nullptr;
        progressDialog->hide();
        return;
    }

    if (data.startsWith("<!DOCTYPE html") || data.startsWith("<html")) {
        qDebug() << "Erreur : Le serveur a renvoyé une page HTML au lieu du fichier.";
        QMessageBox::critical(this, "Erreur", "Le serveur a renvoyé une page HTML au lieu du fichier.");
        reply->deleteLater();
        currentReply = nullptr;
        progressDialog->hide();
        return;
    }

    QFile file(downloadFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Erreur : Impossible d'ouvrir le fichier pour écriture.";
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier pour écriture.");
        reply->deleteLater();
        currentReply = nullptr;
        progressDialog->hide();
        return;
    }

    if (file.write(data) == -1) {
        qDebug() << "Erreur : Impossible d'écrire les données dans le fichier.";
        QMessageBox::critical(this, "Erreur", "Impossible d'écrire les données dans le fichier.");
        file.close();
        reply->deleteLater();
        currentReply = nullptr;
        progressDialog->hide();
        return;
    }

    file.close();
    qDebug() << "Fichier téléchargé avec succès : " << downloadFilePath;

    reply->deleteLater();
    currentReply = nullptr;

    QProcess process;
    process.startDetached(downloadFilePath);
    progressDialog->hide();
    this->hide();
    QCoreApplication::quit;

}


DiagUp::~DiagUp() {
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }
    delete progressDialog; // Ajoutez cette ligne
    delete ui;
}


void DiagUp::on_update_pressed()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(-2, -2); // Décalage de l'ombre (x, y)
    ui->update->setGraphicsEffect(shadowCheck);
}


void DiagUp::on_fermer_released()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(3, 3); // Décalage de l'ombre (x, y)
    ui->fermer->setGraphicsEffect(shadowCheck);
}


void DiagUp::on_update_released()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(3, 3); // Décalage de l'ombre (x, y)
    ui->update->setGraphicsEffect(shadowCheck);
}


void DiagUp::on_fermer_pressed()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(-2, -2); // Décalage de l'ombre (x, y)
    ui->fermer->setGraphicsEffect(shadowCheck);
}


