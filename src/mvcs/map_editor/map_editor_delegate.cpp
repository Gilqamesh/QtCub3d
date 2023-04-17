#include "map_editor_delegate.h"
#include "../map/map_model.h"

#include <QPainter>

Map_Editor_Delegate::Map_Editor_Delegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

void Map_Editor_Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    initStyleOption(const_cast<QStyleOptionViewItem*>(&option), index);
    painter->save();

    bool successful_conversion;
    Map_Model::Cell cell_value = static_cast<Map_Model::Cell>(index.data(Qt::DisplayRole).toInt(&successful_conversion));
    if (successful_conversion == false) {
        painter->restore();
        LOG("successful_conversion == false in 'Map_Editor_Delegate::paint'");
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

QWidget *Map_Editor_Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSpinBox* editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(10);

    return editor;
}

void Map_Editor_Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    bool successful_conversion;
    i32 cell_value = index.model()->data(index, Qt::EditRole).toInt(&successful_conversion);
    if (successful_conversion == false) {
        return ;
    }

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(cell_value);
}

void Map_Editor_Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    i32 value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void Map_Editor_Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}
