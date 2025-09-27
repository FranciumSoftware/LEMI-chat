// customquerymodel.h
#ifndef CUSTOMQUERYMODEL_H
#define CUSTOMQUERYMODEL_H

#include <QSqlQueryModel>

class CustomQueryModel : public QSqlQueryModel {
    Q_OBJECT
public:
    explicit CustomQueryModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // CUSTOMQUERYMODEL_H
