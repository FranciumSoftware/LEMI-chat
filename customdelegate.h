// customdelegate.h
#ifndef CUSTOMDELEGATE_H
#define CUSTOMDELEGATE_H

#include <QStyledItemDelegate>
#include <QString>

class CustomDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit CustomDelegate(const QString &myPseudo, QObject *parent = nullptr);
    void setMyPseudo(const QString &pseudo);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString myPseudo;
};

#endif // CUSTOMDELEGATE_H
