#include "map_model.h"
#include <fstream>

static inline bool checkExtension(const std::string& str, const std::string& extension) {
    if (str.length() < extension.length() + 1) {
        return false;
    }

    size_t ext_start_index = str.find_last_of('.');
    if (ext_start_index == std::string::npos ||
        str.length() - (ext_start_index + 1) != extension.length() ||
        str.substr(ext_start_index + 1) != extension
    ) {
        return false;
    }

    return true;
}

Map_Model::Map_Model(const std::string& cubmap_filepath) {
    if (checkExtension(cubmap_filepath, "cub") == false) {
        throw std::runtime_error("file doesn't have .cub extension");
    }

    std::ifstream ifs(cubmap_filepath);
    if (!ifs) {
        throw std::runtime_error("couldnt open file path for cubmap: '" + cubmap_filepath + "'");
    }
    bool found_player = false;
    std::string line;
    u32 max_row_size = 0;
    while (std::getline(ifs, line)) {
        std::vector<Cell> cur_row;
        if (line.size() > max_row_size) {
            max_row_size = (u32) line.size();
        }
        for (u32 col = 0; col < line.size(); ++col) {
            char c = line[col];
            switch (c) {
                case '0': {
                    cur_row.push_back(Cell::Empty);
                } break ;
                case '1': {
                    cur_row.push_back(Cell::Wall);
                } break ;
                case 'N':
                case 'S':
                case 'W':
                case 'E': {
                    if (found_player) {
                        throw std::runtime_error("found multiple players during parsing the map");
                    }
                    cur_row.push_back(Cell::Player);
                    camera = Camera(
                        (r32) col + 0.5f,
                        (r32) cells.size() + 0.5f,
                        0.0f
                    );
                    found_player = true;
                } break ;
                case ' ': {
                    cur_row.push_back(Cell::Wall);
                } break ;
                default: throw std::runtime_error("unexpected character during parsing map: '" + std::to_string(c) + "'");
            }
        }
        cells.push_back(cur_row);
    }
    for (auto& row : cells) {
        if (row.size() < max_row_size) {
            row.insert(row.end(), max_row_size - row.size(), Cell::Wall);
        }
    }
    if (found_player == false) {
        throw std::runtime_error("didn't find player during parsing the map");
    }
    // todo: validate if closed, has exactly one player
}

int Map_Model::rowCount(const QModelIndex &parent) const {
    return static_cast<i32>(cells.size());
}

int Map_Model::columnCount(const QModelIndex &parent) const {
    if (rowCount(parent) == 0) {
        return 0;
    }
    return static_cast<i32>(cells[0].size());
}

QVariant Map_Model::data(const QModelIndex &index, int role) const {
    if (index.isValid() == false) {
        return QVariant();
    }

    if (isIndexValid(index.column(), index.row()) == false) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return static_cast<i32>(cells[index.row()][index.column()]);
    } else {
        return QVariant();
    }
}

QVariant Map_Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return QStringLiteral("Column %1").arg(section);
    } else {
        return QStringLiteral("Row %1").arg(section);
    }
}


bool Map_Model::isIndexValid(u32 col, u32 row) const {
    return row < cells.size() && col < cells[row].size();
}

bool Map_Model::isPWalkable(u32 col, u32 row) {
    if (isIndexValid(col, row) &&
        cells[row][col] == Map_Model::Cell::Empty ||
        cells[row][col] == Map_Model::Cell::Player
    ) {
        return true;
    }

    return false;
}

bool Map_Model::setData(u32 col, u32 row, Map_Model::Cell value) {
    return setData(createIndex(row, col), QVariant(static_cast<i32>(value)));
}

bool Map_Model::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() == false) {
        return false;
    }

    if (isIndexValid(index.column(), index.row()) == false) {
        return false;
    }

    bool conversion_was_successful;
    Map_Model::Cell cell_value = static_cast<Map_Model::Cell>(value.toInt(&conversion_was_successful));
    if (conversion_was_successful == false) {
        return false;
    }

    if (role == Qt::EditRole && cells[index.row()][index.column()] != cell_value) {
        cells[index.row()][index.column()] = cell_value;
        emit layoutChanged();
        return true;
    }

    return false;
}
