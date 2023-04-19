#ifndef MAP_MODEL_H
#define MAP_MODEL_H

#include "../mvcs_defs.h"

#include "../../camera.h"

#include <QAbstractTableModel>

#include <vector>

class Map_Model: public QAbstractTableModel {
public:
    Map_Model(const std::string& cubmap_filepath);

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

public:
    enum class Cell {
        Empty,
        Player,
        Wall
    };

    Cell getData(u32 col, u32 row) const;
    u32 rowCount() const;
    u32 colCount() const;

    Camera camera;

    bool isIndexValid(u32 col, u32 row) const;
    bool isPWalkable(u32 col, u32 row);

    bool setData(u32 col, u32 row, Map_Model::Cell value);

private:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool isMapValid() const;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

private:
    std::vector<std::vector<Cell>> cells;
};

#endif
