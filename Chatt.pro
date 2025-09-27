QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O0


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QT += sql
QT += network

QMAKE_CXXFLAGS_RELEASE += -g
QMAKE_LFLAGS_RELEASE += -g


SOURCES += \
    customdelegate.cpp \
    customquerymodel.cpp \
    diagup.cpp \
    main.cpp \
    mainwindow.cpp \
    progressdialog.cpp

HEADERS += \
    customdelegate.h \
    customquerymodel.h \
    diagup.h \
    mainwindow.h \
    progressdialog.h

FORMS += \
    diagup.ui \
    mainwindow.ui \
    progressdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources.qrc
