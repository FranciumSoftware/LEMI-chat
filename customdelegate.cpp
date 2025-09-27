// customdelegate.cpp
#include "customdelegate.h"
#include <QPainter>
#include <QDebug>

CustomDelegate::CustomDelegate(const QString &myPseudo, QObject *parent)
    : QStyledItemDelegate(parent), myPseudo(myPseudo) {}

void CustomDelegate::setMyPseudo(const QString &pseudo) {
    myPseudo = pseudo;
    qDebug() << "Nouveau pseudo dans le délégué : " << myPseudo;
}

void CustomDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Récupère le pseudo depuis le texte affiché (format "Pseudo: message")
    QString displayText = index.data(Qt::DisplayRole).toString();
    QString messagePseudo = displayText.split(":").first().trimmed();

    // Définit la couleur de fond en fonction du pseudo
    if (messagePseudo == myPseudo) {
        painter->fillRect(opt.rect, QColor(200, 230, 255)); // Bleu clair pour tes messages
    }

    // Dessine le texte
    QStyledItemDelegate::paint(painter, opt, index);
}
