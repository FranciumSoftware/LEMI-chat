#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "diagup.h"
#include <QStyleFactory>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTimer>
#include "customdelegate.h"
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QTextStream>
#include <QSystemTrayIcon>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    if (!ui) {
        qDebug() << "Erreur : ui est nullptr.";
        return;
    }

    ui->setupUi(this);
    QApplication::setStyle("windowsvista");

    //Styles

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(0); // Rayon de flou de l'ombre
    shadow->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadow->setOffset(5, 5); // Décalage de l'ombre (x, y)
    ui->sendButton->setGraphicsEffect(shadow);
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(3, 3); // Décalage de l'ombre (x, y)
    ui->check->setGraphicsEffect(shadowCheck);

    //end styles




    // Connexion à MySQL
    if (!connectToMySQL()) {
        QMessageBox::critical(this, "Erreur", "Impossible de se connecter à la base de données.");
        this->close();
        return;
    }

    createMessagesTable();

    //Configurer le query model
    queryModel = new CustomQueryModel(this);
    ui->listView->setModel(queryModel);
    delegate = new CustomDelegate(myPseudo, this);
    ui->listView->setItemDelegate(delegate);

    // Configurer le délégué avec votre pseudo
    delegate = new CustomDelegate(myPseudo, this);
    ui->listView->setItemDelegate(delegate);

    ui->tabWidget->setTabIcon(0, QIcon(":/images/assets/chat selected.svg"));
    ui->tabWidget->setTabIcon(1, QIcon(":/images/assets/ic_fluent_settings_24_filled.svg"));

    // Connecter le signal textChanged
    connect(ui->pseudo, &QLineEdit::textChanged, this, &MainWindow::on_pseudo_textChanged);

    firstRefresh();

    // Timer pour le rafraîchissement automatique
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshMessages);
    refreshTimer->start(1000);


    getBans();
    QString aboutText="Lemi (Logiciel d'Entraide via Messages sur Internet)\n"
        "Version 1.1\n"
        "© 2025 Lemi team\n"
        "Appareil: "+QSysInfo::machineHostName()+" "+QSysInfo::productType()+" "+QSysInfo::productVersion()+
        "\nAdresse MAC: "+getMacAddressWindows();
    ui->about->setPlainText(aboutText);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested,
            this, &MainWindow::showListViewContextMenu);
    QFile fichier("pseudo.ini");
    if (!fichier.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Fichier inexistant : on le crée avec une valeur par défaut
        QFile newFile("pseudo.ini");
        if (newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&newFile);
            out << "Guest";
            newFile.close();
            ui->pseudo->setText("Guest");
        } else {
            qWarning() << "Impossible de créer pseudo.ini :" << newFile.errorString();
        }
    } else {
        QTextStream flux(&fichier);
        QString premiereLigne = flux.readLine();
        fichier.close();
        ui->pseudo->setText(premiereLigne);
    }
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Icon.png")); // Remplace par ton icône

    trayMenu = new QMenu(this);
    QAction *restoreAction = new QAction("Restaurer", this);
    QAction *quitAction = new QAction("Quitter", this);

    connect(restoreAction, &QAction::triggered, this, &MainWindow::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayMenu->addAction(restoreAction);
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    ui->textEdit->installEventFilter(this);

    ui->listView->setFlow(QListView::TopToBottom);
    ui->listView->scrollToBottom();

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::showNormal);


}
void MainWindow::restoreFromTray()
{
    qDebug() << "Notification cliquée !";

    this->showNormal();     // Affiche la fenêtre
    this->activateWindow(); // Donne le focus
    this->raise();          // Amène au premier plan
}


void MainWindow::on_pseudo_textChanged(const QString &text) {
    myPseudo = text.trimmed();
    delegate->setMyPseudo(myPseudo);
    ui->listView->update();

    QFile fichier("pseudo.ini");
    if (fichier.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream flux(&fichier);
        flux << myPseudo;
        fichier.close();
    } else {
        qWarning() << "Impossible d'ouvrir pseudo.ini en écriture :" << fichier.errorString();
    }
}



void MainWindow::createMessagesTable() {
    QSqlQuery query;
}

void MainWindow::insertMessage(const QString &pseudo, const QString &message) {
    QSqlQuery query;
    query.prepare("INSERT INTO messages (pseudo, contenu, mac) VALUES (:pseudo, :contenu, :mac)");
    query.bindValue(":pseudo", pseudo);  // Utiliser l'argument `pseudo`
    query.bindValue(":contenu", message); // Utiliser l'argument `message`
    query.bindValue(":mac", getMacAddressWindows());
    if (!query.exec()) {
        qDebug() << "Erreur lors de l'insertion du message : " << query.lastError().text();
    }
}


void MainWindow::refreshMessages() {
    int legth=ui->listView->model()->rowCount();
    queryModel->setQuery("SELECT pseudo, contenu FROM messages ORDER BY date_envoi ASC");

    if (queryModel->lastError().isValid()) {
        qDebug() << "Erreur lors du chargement des messages : " << queryModel->lastError().text();
    }
    int nlength=ui->listView->model()->rowCount();
    int mid=nlength-legth;
    if (!this->isActiveWindow()){
        if (legth<nlength){
            QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
            trayIcon->setIcon(QIcon(":/Icon.png")); // Icône de la notification
            trayIcon->show();


            int lastRow = queryModel->rowCount() - 1;
            QString dernierMessage;

            if (lastRow >= 0) {
                QModelIndex index = queryModel->index(lastRow, 1); // colonne 1 = contenu
                dernierMessage = index.data().toString();
            }

            trayIcon->showMessage("Nouveaux messages", dernierMessage, QSystemTrayIcon::Information, 15000);
            connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::showNormal);

        }
    }
    ui->listView->scrollToBottom();
}
void MainWindow::firstRefresh(){
    queryModel->setQuery("SELECT pseudo, contenu FROM messages ORDER BY date_envoi ASC");

    if (queryModel->lastError().isValid()) {
        qDebug() << "Erreur lors du chargement des messages : " << queryModel->lastError().text();
    }
    ui->listView->scrollToBottom();

}

void MainWindow::on_sendButton_clicked() {
    // Récupérer le pseudo et le message
    QString pseudo = ui->pseudo->text().trimmed(); // Remplacez "pseudoLineEdit" par le nom réel de votre champ
    QString message = ui->textEdit->toPlainText().trimmed();

    // Vérifier que ni le pseudo ni le message ne sont vides
    if (!pseudo.isEmpty() && !message.isEmpty()) {
        insertMessage(pseudo, message); // Appeler la méthode avec les deux arguments
        ui->textEdit->clear(); // Effacer le champ de message
        refreshMessages(); // Rafraîchir la liste des messages
    } else {
        QMessageBox::warning(this, "Erreur", "Le pseudo et le message ne peuvent pas être vides.");
    }
}
void MainWindow::getBans() {
    QSqlQuery query("SELECT mac, message, jours FROM ban");
    macList.clear();
    messageList.clear();
    joursList.clear();
    while (query.next()) {
        QString mac = query.value("mac").toString().trimmed().toUpper();
        QString message = query.value("message").toString().trimmed();
        QString jours = query.value("jours").toString().trimmed();
        macList.append(mac);
        messageList.append(message);
        joursList.append(jours);
    }

    QString mymac = getMacAddressWindows();
    if (macList.contains(mymac)) {
        int index = macList.indexOf(mymac);
        if (index >= 0 && index < messageList.size() && index < joursList.size()) {
            QString avertissement = "Vous avez été banni car un ou des messages que vous avez écrits contiennent des propos offensants. "
                                    "\nMessage concerné: " + messageList[index] +
                                    "\nDurée du bannissement: " + joursList[index] + " jours";
            QMessageBox messageBox(QMessageBox::Warning,
                                   "Vous êtes banni",
                                   avertissement,
                                   QMessageBox::Ok);
            messageBox.exec();

            // Utiliser QTimer::singleShot pour quitter après la fermeture de la boîte de dialogue
            QTimer::singleShot(0, qApp, &QApplication::quit);
        }
    }
}


QString MainWindow::getMacAddressWindows() {
    QProcess process;
    process.start("ipconfig", QStringList() << "/all");
    process.waitForFinished(-1); // Attendre la fin du processus

    QString output = process.readAllStandardOutput();
    QRegularExpression re("([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})");

    QRegularExpressionMatchIterator it = re.globalMatch(output);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.hasMatch()) {
            qDebug ()<<"Adresse MAC: "<<match.captured(0);
            return match.captured(0); // Retourne la première adresse MAC trouvée
        }
    }

    return QString(); // Retourne une chaîne vide si aucune adresse MAC n'est trouvée
}

QString MainWindow::normalizeMac(const QString &mac) {
    return mac.trimmed().toUpper().replace(":", "-");
}
void MainWindow::on_check_clicked()
{
    DiagUp check;
    check.exec();
}
void MainWindow::showListViewContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    QAction *actionSignaler = menu.addAction("Signaler");

    QAction *selectedAction = menu.exec(ui->listView->viewport()->mapToGlobal(pos));
    if (selectedAction == actionSignaler) {
        QString message = queryModel->data(index, Qt::DisplayRole).toString();

        // Ajoutez ici la logique pour signaler le message
        QSqlQuery query;
        query.prepare("INSERT INTO `LEMIMain_vastengine`.`flags` (`Message`, `mac`, `pseudo`) VALUES (:contenu, :mac, :pseudo)");
        query.bindValue(":pseudo", myPseudo);  // Utiliser l'argument `pseudo`
        query.bindValue(":contenu", message); // Utiliser l'argument `message`
        query.bindValue(":mac", getMacAddressWindows());
        if (!query.exec()) {
            qDebug() << "Erreur lors de l'insertion du message : " << query.lastError().text();
        }
    }
}

MainWindow::~MainWindow() {
    QSqlDatabase::database().close();
    delete ui;
}





void MainWindow::on_sendButton_pressed()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(0); // Rayon de flou de l'ombre
    shadow->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadow->setOffset(1, 1); // Décalage de l'ombre (x, y)
    ui->sendButton->setGraphicsEffect(shadow);
}


void MainWindow::on_sendButton_released()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(0); // Rayon de flou de l'ombre
    shadow->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadow->setOffset(5, 5); // Décalage de l'ombre (x, y)
    ui->sendButton->setGraphicsEffect(shadow);
}


void MainWindow::on_tabWidget_currentChanged(int index) {

    // Réinitialise toutes les icônes à leur état par défaut
    ui->tabWidget->setTabIcon(0, QIcon(":/images/assets/ic_fluent_chat_multiple_24_filled.svg"));
    ui->tabWidget->setTabIcon(1, QIcon(":/images/assets/ic_fluent_settings_24_filled.svg"));

    // Change l'icône de l'onglet sélectionné
    if (index == 0) {
        ui->tabWidget->setTabIcon(0, QIcon(":/images/assets/chat selected.svg"));
    } else if (index == 1) {
        ui->tabWidget->setTabIcon(1, QIcon(":/images/assets/setting selected.svg"));
    }
}



void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if (index==0) {

        ui->tabWidget->setTabIcon(0, QIcon(":/images/assets/chat selected.svg"));
        ui->tabWidget->setTabIcon(1, QIcon(":/images/assets/ic_fluent_settings_24_filled.svg"));

    }else if (index==0){
        ui->tabWidget->setTabIcon(0, QIcon(":/images/assets/ic_fluent_settings_24_filled.svg"));
        ui->tabWidget->setTabIcon(1, QIcon(":/images/assets/setting selected.svg"));
    }
}


void MainWindow::on_check_pressed()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(-2, -2); // Décalage de l'ombre (x, y)
    ui->check->setGraphicsEffect(shadowCheck);
}


void MainWindow::on_check_released()
{
    QGraphicsDropShadowEffect *shadowCheck = new QGraphicsDropShadowEffect;
    shadowCheck->setBlurRadius(0); // Rayon de flou de l'ombre
    shadowCheck->setColor(QColor(0, 0, 0, 255)); // Couleur noire avec transparence
    shadowCheck->setOffset(3, 3); // Décalage de l'ombre (x, y)
    ui->check->setGraphicsEffect(shadowCheck);
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    setVisible(false); // Cache proprement la fenêtrehide(); // Cache la fenêtre

    event->ignore(); // Empêche la fermeture
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            on_sendButton_clicked(); // Appelle ta fonction d'envoi
            return true; // Empêche le retour à la ligne
        }
    }
    return QMainWindow::eventFilter(obj, event);
}


