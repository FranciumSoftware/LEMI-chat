#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QTimer>
#include <QSettings>
#include "customquerymodel.h"
#include "customdelegate.h"
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void firstRefresh();
    void refreshMessages();
    bool connectToMySQL();
    void createMessagesTable();
    void getBans();
    void insertMessage(const QString &pseudo, const QString &message);
    QString normalizeMac(const QString &mac);

private slots:
    void on_sendButton_clicked();
    void on_pseudo_textChanged(const QString &text); // Doit correspondre au nom du slot
    void showListViewContextMenu(const QPoint &pos);
    void on_check_clicked();
    void restoreFromTray();
    void on_sendButton_pressed();

    void on_sendButton_released();

    void on_tabWidget_currentChanged(int index);

    void on_tabWidget_tabBarClicked(int index);

    void on_check_pressed();

    void on_check_released();

private:
    Ui::MainWindow *ui;
    QStringListModel *model;
    QSqlDatabase db;
    QTimer *refreshTimer;
    CustomQueryModel *queryModel;
    QString myPseudo;
    CustomDelegate *delegate;
    QStringList macList;    // Liste pour les adresses MAC
    QStringList messageList; // Liste pour les messages
    QStringList joursList;   // Liste pour les jours
    QString getMacAddressWindows();
    QSettings settings;
    QSystemTrayIcon *trayIcon=nullptr;
    QMenu *trayMenu;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // MAINWINDOW_H
