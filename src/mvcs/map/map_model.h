#ifndef MAP_MODEL_H
#define MAP_MODEL_H

#include "../mvcs_defs.h"

#include "../../camera.h"

#include <QAbstractTableModel>

#include <vector>

class Map_Model: public QAbstractTableModel {
public:
    Map_Model(const std::string& cubmap_filepath);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public:
    enum class Cell {
        Empty,
        Player,
        Wall
    };

    Camera camera;
    std::vector<std::vector<Cell>> cells;

    bool isIndexValid(u32 col, u32 row) const;
    bool isPWalkable(u32 col, u32 row);

    bool setData(u32 col, u32 row, Map_Model::Cell value);

private:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
