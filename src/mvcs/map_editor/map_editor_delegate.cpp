#include "map_editor_delegate.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

bool Map_Editor_Delegate::tryToConvert(const QModelIndex& index, Map_Model::Cell& value, i32 role) const {
    bool conversion_result;
    value = static_cast<Map_Model::Cell>(index.data(Qt::DisplayRole).toInt(&conversion_result));
    return conversion_result;
}

Map_Editor_Delegate::Map_Editor_Delegate(QObject *parent)
    : QAbstractItemDelegate(parent) {
    is_mouse_down = false;
    is_valid = false;
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
        case QEvent::MouseButtonPress: {
            QMouseEvent* m_event = dynamic_cast<QMouseEvent*>(event);
            if (m_event->buttons() & Qt::LeftButton) {
                is_mouse_down = true;
                if (tryToConvert(index, new_cell_value, Qt::EditRole) == false) {
                    is_valid = false;
                    return false;
                }
                if (new_cell_value == Map_Model::Cell::Empty) {
                    new_cell_value = Map_Model::Cell::Wall;
                } else if (new_cell_value == Map_Model::Cell::Wall) {
                    new_cell_value = Map_Model::Cell::Empty;
                }
                is_valid = true;
            }
        } break ;
        case QEvent::MouseButtonRelease: {
            QMouseEvent* m_event = dynamic_cast<QMouseEvent*>(event);
            if (m_event->buttons() & Qt::LeftButton) {
                is_mouse_down = false;
            }
        } break ;
        case QEvent::MouseMove: {
            if (is_mouse_down && is_valid) {
                return reinterpret_cast<Map_Model*>(model)->setData(index.column(), index.row(), new_cell_value);
            }
            return true;
        } break ;
        default: {
            return QAbstractItemDelegate::editorEvent(event, model, option, index);
        }
    }
}
