
#ifndef CUBMAP_MODEL_H
#define CUBMAP_MODEL_H

#include "defs.h"
#include <QAbstractTableModel>
#include <vector>
#include "v2.h"

class CubMap_Model : public QAbstractTableModel
{
public:
    enum class CubMap_ModelCell {
        Empty,
        Outside,
        Player,
        Wall
    };

public:
    CubMap_Model(const string& cub_file_path);

    CubMap_ModelCell& operator()(u32 row, u32 col);

private:
    v2<r32> _player_p;
    vector<vector<CubMap_ModelCell>> _map;

    bool isIndexValid(u32 row, u32 col) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // CUBMAP_MODEL_H
