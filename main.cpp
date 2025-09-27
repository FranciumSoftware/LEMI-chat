#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile styleFile(":/styles.qss"); // Assurez-vous que le fichier est accessible dans les ressources Qt
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }
    MainWindow w;
    w.show();
    return a.exec();
}
