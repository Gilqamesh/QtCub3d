#include "cubmap_delegate.h"
#include <QPainter>
#include "cubmap_model.h"

CubMap_Delegate::CubMap_Delegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

void CubMap_Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    initStyleOption(const_cast<QStyleOptionViewItem*>(&option), index);
    painter->save();

    bool successful_conversion;
    CubMap_Model::CubMap_ModelCell cell_value = static_cast<CubMap_Model::CubMap_ModelCell>(index.data(Qt::DisplayRole).toInt(&successful_conversion));
    if (successful_conversion == false) {
        return ;
    }
    switch (cell_value) {
        case CubMap_Model::CubMap_ModelCell::Empty: {
            painter->fillRect(option.rect, Qt::white);
        } break ;
        case CubMap_Model::CubMap_ModelCell::Outside: {
            painter->fillRect(option.rect, Qt::black);
        } break ;
        case CubMap_Model::CubMap_ModelCell::Wall: {
            painter->fillRect(option.rect, Qt::blue);
        } break ;
        default: throw std::runtime_error("not implemented");
    }

    painter->restore();
}

QWidget *CubMap_Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSpinBox* editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(10);

    return editor;
}

void CubMap_Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    bool successful_conversion;
    i32 cell_value = index.model()->data(index, Qt::EditRole).toInt(&successful_conversion);
    if (successful_conversion == false) {
        return ;
    }

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(cell_value);
}

void CubMap_Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    i32 value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void CubMap_Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}
