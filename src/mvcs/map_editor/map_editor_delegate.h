#ifndef MAP_EDITOR_DELEGATE_H
#define MAP_EDITOR_DELEGATE_H

#include "../mvcs_defs.h"

#include "../map/map_model.h"

#include <QSpinBox>
#include <QStyledItemDelegate>

class Map_Editor_Delegate : public QAbstractItemDelegate {
public:
    Map_Editor_Delegate(QObject *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    bool tryToConvert(const QModelIndex& index, Map_Model::Cell& value, i32 role) const;

    bool is_mouse_down;
    bool is_valid;
    Map_Model::Cell new_cell_value;
};

#endif // MAP_EDITOR_DELEGATE_H
