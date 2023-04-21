#ifndef MAP_EDITOR_VIEW_H
#define MAP_EDITOR_VIEW_H

#include "../mvcs_defs.h"
#include "../map/map_model.h"

#include "map_editor_delegate.h"

#include <QTableView>

class Map_Editor_View: public QTableView {
public:
    Map_Editor_View(Map_Model* map_model, QWidget *parent = nullptr);

public slots:
    void centerAround(const QModelIndex &index);

private:
    Map_Editor_Delegate map_editor_delegate;
};

#endif
