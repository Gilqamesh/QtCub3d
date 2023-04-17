#ifndef MAP_EDITOR_DELEGATE_H
#define MAP_EDITOR_DELEGATE_H

#include "../mvcs_defs.h"

#include <QSpinBox>
#include <QStyledItemDelegate>

class Map_Editor_Delegate : public QStyledItemDelegate {
public:
    Map_Editor_Delegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // MAP_EDITOR_DELEGATE_H
