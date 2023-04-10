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
    while (getline(ifs, line)) {
        vector<CubMap_ModelCell> cur_row;
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
                    cur_row.push_back(CubMap_ModelCell::Player);
                    _player_p.x = (r32) _map.size() + 0.5f;
                    _player_p.y = (r32) col + 0.5f;
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
    // todo: validate if closed, has exactly one player
}

CubMap_Model::CubMap_ModelCell& CubMap_Model::operator()(u32 row, u32 col) {
    return _map[row][col];
}

bool CubMap_Model::isIndexValid(u32 row, u32 col) const {
    return !(row >= _map.size() || col >= _map[row].size());
}

int CubMap_Model::rowCount(const QModelIndex &parent) const {
    return static_cast<i32>(_map.size());
}

int CubMap_Model::columnCount(const QModelIndex &parent) const {
    if (isIndexValid(parent.row(), 0) == false) {
        return 0;
    }
    return static_cast<i32>(_map[parent.row()].size());
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
