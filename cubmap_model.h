
#ifndef CUBMAP_MODEL_H
#define CUBMAP_MODEL_H

#include "defs.h"
#include <QAbstractTableModel>
#include "camera.h"
#include <vector>
#include "v2.h"

class CubMap_Model : public QAbstractTableModel
{
public:
    enum class CubMap_ModelCell {
        Empty,
        Outside,
        Wall
    };

public:
    CubMap_Model(const std::string& cub_file_path);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public:
    Camera _camera;
    std::vector<std::vector<CubMap_ModelCell>> _map;

    bool isIndexValid(u32 col, u32 row) const;
};

#endif // CUBMAP_MODEL_H
