#ifndef CUBMAP_DELEGATE_H
#define CUBMAP_DELEGATE_H

#include <QSpinBox>
#include <QStyledItemDelegate>
#include "defs.h"

class CubMap_Delegate : public QStyledItemDelegate
{
public:
    CubMap_Delegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CUBMAP_DELEGATE_H
