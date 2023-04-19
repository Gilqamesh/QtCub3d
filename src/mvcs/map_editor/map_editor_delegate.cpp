#include "map_editor_delegate.h"
#include "../map/map_model.h"

#include <QPainter>
#include <QEvent>

bool Map_Editor_Delegate::tryToConvert(const QModelIndex& index, Map_Model::Cell& value, i32 role) const {
    bool conversion_result;
    value = static_cast<Map_Model::Cell>(index.data(Qt::DisplayRole).toInt(&conversion_result));
    return conversion_result;
}

Map_Editor_Delegate::Map_Editor_Delegate(QObject *parent)
    : QAbstractItemDelegate(parent) {
}

void Map_Editor_Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    Map_Model::Cell cell_value;
    if (tryToConvert(index, cell_value, static_cast<i32>(Qt::DisplayRole)) == false) {
        painter->restore();
        return ;
    }
    switch (cell_value) {
        case Map_Model::Cell::Empty: {
            painter->fillRect(option.rect, Qt::white);
        } break ;
        case Map_Model::Cell::Player: {
            painter->fillRect(option.rect, Qt::red);
        } break ;
        case Map_Model::Cell::Wall: {
            painter->fillRect(option.rect, Qt::blue);
        } break ;
        default: throw std::runtime_error("not implemented");
    }

    painter->restore();
}

QSize Map_Editor_Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return option.decorationSize;
}

bool Map_Editor_Delegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    switch (event->type()) {
        case QEvent::MouseButtonDblClick: {
            Map_Model::Cell cell_value;
            if (tryToConvert(index, cell_value, Qt::EditRole) == false) {
                LOG("Failed to convert in 'editorEvent' " << index.row() << " " << index.column());
            }
            if (cell_value == Map_Model::Cell::Empty) {
                cell_value = Map_Model::Cell::Wall;
            } else if (cell_value == Map_Model::Cell::Wall) {
                cell_value = Map_Model::Cell::Empty;
            }
            reinterpret_cast<Map_Model*>(model)->setData(index.column(), index.row(), cell_value);
            return true;
        } break ;
        default: {
            return QAbstractItemDelegate::editorEvent(event, model, option, index);
        }
    }
}
