#include "cubmap_model.h"

#include <fstream>

CubMap_Model::CubMap_Model(const string& cub_file_path)
{
    ifstream ifs(cub_file_path);
    if (!ifs) {
        throw runtime_error("couldnt open file path for cubmap: '" + cub_file_path + "'");
    }
    bool found_player = false;
    string line;
    u32 max_row_size = 0;
    while (getline(ifs, line)) {
        vector<CubMap_ModelCell> cur_row;
        if (line.size() > max_row_size) {
            max_row_size = (u32) line.size();
        }
        for (u32 col = 0; col < line.size(); ++col) {
            char c = line[col];
            switch (c) {
                case '0': {
                    cur_row.push_back(CubMap_ModelCell::Empty);
                } break ;
                case '1': {
                    cur_row.push_back(CubMap_ModelCell::Wall);
                } break ;
                case 'N':
                case 'S':
                case 'W':
                case 'E': {
                    if (found_player) {
                        throw runtime_error("found multiple players during parsing the map");
                    }
                    cur_row.push_back(CubMap_ModelCell::Empty);
                    _camera = Camera(
                        (r32) col + 0.5f,
                        (r32) _map.size() + 0.5f,
                        0.0f
                    );
                    found_player = true;
                } break ;
                case ' ': {
                    cur_row.push_back(CubMap_ModelCell::Outside);
                } break ;
                default: throw runtime_error("unexpected character during parsing map: '" + to_string(c) + "'");
            }
        }
        _map.push_back(cur_row);
    }
    for (auto& row : _map) {
        if (row.size() < max_row_size) {
            row.insert(row.end(), max_row_size - row.size(), CubMap_ModelCell::Outside);
        }
    }
    if (found_player == false) {
        throw runtime_error("didn't find player during parsing the map");
    }
    // todo: validate if closed, has exactly one player
}

bool CubMap_Model::isIndexValid(u32 col, u32 row) const {
    return !(row >= _map.size() || col >= _map[row].size());
}

int CubMap_Model::rowCount(const QModelIndex &parent) const {
    return static_cast<i32>(_map.size());
}

int CubMap_Model::columnCount(const QModelIndex &parent) const {
    if (rowCount(parent) == 0) {
        return 0;
    }
    return static_cast<i32>(_map[0].size());
}

QVariant CubMap_Model::data(const QModelIndex &index, int role) const {
    if (index.isValid() == false) {
        return QVariant();
    }

    if (isIndexValid(index.row(), index.column()) == false) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return static_cast<i32>(_map[index.row()][index.column()]);
    } else {
        return QVariant();
    }
}

QVariant CubMap_Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QStringLiteral("Column %1").arg(section);
    } else {
        return QStringLiteral("Row %1").arg(section);
    }
}
