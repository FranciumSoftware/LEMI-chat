// customquerymodel.cpp
#include "customquerymodel.h"

CustomQueryModel::CustomQueryModel(QObject *parent) : QSqlQueryModel(parent) {}

QVariant CustomQueryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        QString pseudo = QSqlQueryModel::data(this->index(index.row(), 0)).toString();
        QString contenu = QSqlQueryModel::data(this->index(index.row(), 1)).toString();
        return pseudo + ": " + contenu; // Format "Pseudo: message"
    }
    return QVariant();
}
