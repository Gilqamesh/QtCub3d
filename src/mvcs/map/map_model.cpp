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

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
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

Qt::ItemFlags Map_Model::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool Map_Model::insertRows(int row, int count, const QModelIndex &parent) {
    assert(row >= 0 && count >= 0);

    if (row >= rowCount(parent)) {
        row = rowCount(parent) - 1;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);
    cells.insert(cells.begin() + row, count, std::vector<Cell>(columnCount(parent), Cell::Wall));
    endInsertRows();

    return true;
}

bool Map_Model::insertColumns(int column, int count, const QModelIndex &parent) {
    assert(column >= 0 && count >= 0);

    if (column > columnCount(parent)) {
        column = columnCount(parent);
    }
    beginInsertColumns(QModelIndex(), column, column + count - 1);
    for (auto& row: cells) {
        row.insert(row.begin() + column, count, Cell::Wall);
    }
    endInsertColumns();

    return true;
}

bool Map_Model::removeRows(int row, int count, const QModelIndex &parent) {
    assert(row >= 0 && count >= 0);

    if (rowCount(parent) == 0) {
        return false;
    }

    if (row >= rowCount(parent)) {
        row = rowCount(parent) - 1;
    }
    if (row + count >= rowCount(parent)) {
        count = rowCount(parent) - 1 - row;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    cells.erase(cells.begin() + row, cells.begin() + row + count);
    endRemoveRows();

    return true;
}

bool Map_Model::removeColumns(int column, int count, const QModelIndex &parent) {
    assert(column >= 0 && count >= 0);

    if (columnCount(parent) == 0) {
        return false;
    }

    if (column >= columnCount(parent)) {
        column = columnCount(parent) - 1;
    }
    if (column + count >= columnCount(parent)) {
        count = columnCount(parent) - 1 - column;
    }

    beginRemoveColumns(QModelIndex(), column, column + count - 1);
    for (auto& row: cells) {
        row.erase(row.begin() + column, row.begin() + column + count);
    }
    endRemoveColumns();

    return true;
}

Map_Model::Cell Map_Model::getData(u32 col, u32 row) const {
    return cells[row][col];
}

u32 Map_Model::rowCount() const {
    return static_cast<u32>(cells.size());
}

u32 Map_Model::colCount() const {
    if (rowCount() == 0) {
        return 0;
    }

    return static_cast<u32>(cells[0].size());
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

    Map_Model::Cell& old_cell_value = cells[index.row()][index.column()];
    if (role == Qt::EditRole && old_cell_value != cell_value) {

        cells[index.row()][index.column()] = cell_value;
        if (isMapValid() == false) {
            cells[index.row()][index.column()] = old_cell_value;

            return false;
        } else {
            emit dataChanged(index, index, {role});

            return true;
        }
    }

    return false;
}

bool Map_Model::isMapValid() const {
    if (cells.size() == 0 || cells[0].size() == 0) {
        return false;
    }

    for (u32 col = 0; col < cells[0].size(); ++col) {
        if (cells[0][col] != Cell::Wall) {
            return false;
        }
        if (cells[cells.size() - 1][col] != Cell::Wall) {
            return false;
        }
    }
    for (u32 row = 1; row < cells.size() - 1; ++row) {
        if (cells[row][0] != Cell::Wall) {
            return false;
        }
        if (cells[row][cells[row].size() - 1] != Cell::Wall) {
            return false;
        }
    }

    return true;
}
