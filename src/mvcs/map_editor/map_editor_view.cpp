#include "map_editor_view.h"

#include <QHeaderView>

Map_Editor_View::Map_Editor_View(Map_Model* map_model, QWidget *parent)
    : QTableView(parent) {
    setModel(map_model);
    setItemDelegate(&map_editor_delegate);

    QHeaderView* horizontal_header = this->horizontalHeader();
    QHeaderView* vertical_header = this->verticalHeader();
    horizontal_header->hide();
    vertical_header->hide();
    horizontal_header->setDefaultSectionSize(20);
    vertical_header->setDefaultSectionSize(20);

    this->setShowGrid(false);
    this->setSelectionMode(QAbstractItemView::NoSelection);
}

void Map_Editor_View::centerAround(const QModelIndex &index) {
    scrollTo(index, QAbstractItemView::PositionAtCenter);
}
