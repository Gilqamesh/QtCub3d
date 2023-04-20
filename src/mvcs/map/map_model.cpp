#include "map_model.h"
#include <fstream>

#include <QMessageBox>

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
    readMap(cubmap_filepath);
}

void Map_Model::readMap(const std::string& cubmap_filepath) {
    if (checkExtension(cubmap_filepath, "cub") == false) {
        throw std::runtime_error("file doesn't have .cub extension");
    }

    std::ifstream ifs(cubmap_filepath);
    if (!ifs) {
        throw std::runtime_error("couldnt open file path for cubmap: '" + cubmap_filepath + "'");
    }

    std::vector<std::vector<Cell>> cur_cells;
    bool found_player = false;
    std::string line;
    u32 max_row_size = 0;
    while (std::getline(ifs, line)) {
        std::vector<Cell> cur_row;
        if (line.size() > max_row_size) {
            max_row_size = (u32) line.size();
        }
        for (u32 col = 0; col < line.size(); ++col) {
            Cell cell = charToCell(line[col]);
            if (cell == Cell::Player) {
                if (found_player) {
                    throw std::runtime_error("found multiple players during parsing the map: " + cubmap_filepath);
                }
                camera = Camera(
                    (r32) col + 0.5f,
                    (r32) cur_cells.size() + 0.5f,
                    0.0f
                );
                found_player = true;
            }
            cur_row.push_back(cell);
        }
        cur_cells.push_back(cur_row);
    }
    for (auto& row : cur_cells) {
        if (row.size() < max_row_size) {
            row.insert(row.end(), max_row_size - row.size(), Cell::Wall);
        }
    }
    if (found_player == false) {
        throw std::runtime_error("didn't find player during parsing the map: " + cubmap_filepath);
    }

    if (isMapEnclosed(cur_cells) == false) {
        throw std::runtime_error("map is not enclosed: " + cubmap_filepath);
    }
    cells = cur_cells;
}

void Map_Model::saveMap(const std::string& cubmap_filepath) {
    std::ofstream savedFile(cubmap_filepath);
    if (!savedFile) {
        throw std::runtime_error("Unable to open file for writing");
    }
    for (u32 row = 0; row < rowCount(); ++row) {
        for (u32 col = 0; col < colCount(); ++col) {
            savedFile << cellToChar(getData(col, row));
        }
        savedFile << '\n';
    }
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

Map_Model::Cell Map_Model::charToCell(char c) const {
    switch (c) {
        case '0': {
            return Cell::Empty;
        } break ;
        case '1': {
            return Cell::Wall;
        } break ;
        case 'N':
        case 'S':
        case 'W':
        case 'E': {
            return Cell::Player;
        } break ;
        case ' ': {
            return Cell::Wall;
        } break ;
        default: {
            throw std::runtime_error("unexpected character");
        }
    }

    return Cell::Empty;
}

char Map_Model::cellToChar(Cell cell) const {
    switch (cell) {
        case Cell::Empty: {
            return '0';
        } break ;
        case Cell::Wall: {
            return '1';
        } break ;
        case Cell::Player: {
            return 'N';
        } break ;
        default: {
            throw std::runtime_error("unexpected cell value");
        }
    }

    return '\0';
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
    return setData(createIndex(row, col), QVariant(static_cast<i32>(value)), Qt::EditRole);
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

    Map_Model::Cell old_cell_value = cells[index.row()][index.column()];
    if (role == Qt::EditRole && old_cell_value != cell_value) {

        cells[index.row()][index.column()] = cell_value;
        if (isMapEnclosed(cells) == false) {
            cells[index.row()][index.column()] = old_cell_value;

            return false;
        } else {
            emit dataChanged(index, index, {role});

            return true;
        }
    }

    return true;
}

bool Map_Model::isMapEnclosed(const std::vector<std::vector<Cell>>& m) const {
    if (m.size() == 0 || m[0].size() == 0) {
        return false;
    }

    for (u32 col = 0; col < m[0].size(); ++col) {
        if (m.back()[col] != Cell::Wall ||
            m.front()[col] != Cell::Wall
        ) {
            return false;
        }
    }
    for (u32 row = 1; row < m.size() - 1; ++row) {
        if (m[row].front() != Cell::Wall ||
            m[row].back() != Cell::Wall
        ) {
            return false;
        }
    }

    return true;
}
