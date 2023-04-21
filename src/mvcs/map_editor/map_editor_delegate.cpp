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
    is_cell_valid = false;

    std::string wall_tex_path = PROJECT_DIR"assets/WallIcon.png";
    wall_tex = QImage(wall_tex_path.c_str());
    if (wall_tex.isNull()) {
        throw std::runtime_error("failed to load wall_tex_path: " + wall_tex_path);
    }
    std::string player_tex_path = PROJECT_DIR"assets/PlayerIcon";
    player_tex = QImage(player_tex_path.c_str());
    if (player_tex.isNull()) {
        throw std::runtime_error("failed to load player_tex_path: " + player_tex_path);
    }
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
            painter->drawImage(option.rect, player_tex);
            // painter->fillRect(option.rect, Qt::red);
        } break ;
        case Map_Model::Cell::Wall: {
            painter->drawImage(option.rect, wall_tex);
            // painter->fillRect(option.rect, Qt::blue);
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
                is_cell_valid = false;
                if (tryToConvert(index, new_cell_value, Qt::EditRole) == false) {
                    return false;
                }
                if (new_cell_value == Map_Model::Cell::Empty) {
                    new_cell_value = Map_Model::Cell::Wall;
                    is_cell_valid = true;
                    reinterpret_cast<Map_Model*>(model)->setData(index.column(), index.row(), new_cell_value);
                } else if (new_cell_value == Map_Model::Cell::Wall) {
                    new_cell_value = Map_Model::Cell::Empty;
                    is_cell_valid = true;
                    reinterpret_cast<Map_Model*>(model)->setData(index.column(), index.row(), new_cell_value);
                }
            }
        } break ;
        case QEvent::MouseButtonRelease: {
            QMouseEvent* m_event = dynamic_cast<QMouseEvent*>(event);
            if (m_event->buttons() & Qt::LeftButton) {
                is_mouse_down = false;
            }
        } break ;
        case QEvent::MouseMove: {
            if (is_mouse_down && is_cell_valid) {
                if (reinterpret_cast<Map_Model*>(model)->getData(index.column(), index.row()) == Map_Model::Cell::Player) {
                    return false;
                }
                return reinterpret_cast<Map_Model*>(model)->setData(index.column(), index.row(), new_cell_value);
            }
        } break ;
        default: {
            return QAbstractItemDelegate::editorEvent(event, model, option, index);
        }
    }

    return true;
}
