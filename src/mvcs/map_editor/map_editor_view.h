#ifndef MAP_EDITOR_VIEW_H
#define MAP_EDITOR_VIEW_H

#include "../mvcs_defs.h"

#include <QTableView>

class Map_Editor_View : public QTableView {
public:
    Map_Editor_View(QWidget *parent = nullptr);
};

#endif // MAP_EDITOR_VIEW_H
